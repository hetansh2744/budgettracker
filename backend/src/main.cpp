#include "httplib.h"
#include "json.hpp"
#include "Env.hpp"
#include "Db.hpp"
#include "Password.hpp"
#include "Jwt.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <libpq-fe.h>

using json = nlohmann::json;

static std::string readFile(const std::string& path) {
  std::ifstream f(path);
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

static void addCors(httplib::Response& res, const std::string& origin) {
  if (origin.empty()) return;
  res.set_header("Access-Control-Allow-Origin", origin.c_str());
  res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
  res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
}

static void jsonError(httplib::Response& res, int status, const std::string& code, const std::string& msg, const std::string& origin) {
  addCors(res, origin);
  res.status = status;
  res.set_content(json({{"error", {{"code", code}, {"message", msg}}}}).dump(), "application/json");
}

static long requireAuth(const httplib::Request& req, httplib::Response& res, const std::string& jwtSecret, const std::string& origin) {
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) {
    jsonError(res, 401, "UNAUTHORIZED", "Missing Authorization header", origin);
    return 0;
  }
  const std::string prefix = "Bearer ";
  const std::string& h = it->second;
  if (h.rfind(prefix, 0) != 0) {
    jsonError(res, 401, "UNAUTHORIZED", "Invalid Authorization header", origin);
    return 0;
  }
  auto userId = Jwt::verifyAndGetUserId(h.substr(prefix.size()), jwtSecret);
  if (!userId) {
    jsonError(res, 401, "UNAUTHORIZED", "Invalid or expired token", origin);
    return 0;
  }
  return *userId;
}

int main() {
  try {
    const int port = Env::getInt("PORT", 10000);
    const std::string host = "0.0.0.0";

    const std::string dbUrl = Env::get("DATABASE_URL");
    if (dbUrl.empty()) {
      std::cerr << "DATABASE_URL is required\n";
      return 1;
    }

    const std::string jwtSecret = Env::get("JWT_SECRET");
    if (jwtSecret.size() < 16) {
      std::cerr << "JWT_SECRET must be set (>=16 chars)\n";
      return 1;
    }

    // Put your frontend URL here on Render (or use "*" for quick testing)
    const std::string corsOrigin = Env::get("CORS_ORIGIN", "*");

    Db db(dbUrl);
    db.execOrThrow(readFile("migrations.sql"));

    httplib::Server srv;

    // OPTIONS preflight
    srv.Options(R"(.*)", [&](const httplib::Request&, httplib::Response& res) {
      addCors(res, corsOrigin);
      res.status = 204;
    });

    // Health
    srv.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
      addCors(res, corsOrigin);
      res.set_content(R"({"ok":true})", "application/json");
    });

    // Register
    srv.Post("/auth/register", [&](const httplib::Request& req, httplib::Response& res) {
      auto body = json::parse(req.body, nullptr, false);
      if (body.is_discarded()) return jsonError(res, 400, "BAD_JSON", "Invalid JSON", corsOrigin);

      std::string name = body.value("name", "");
      std::string email = body.value("email", "");
      std::string password = body.value("password", "");

      if (name.empty() || email.empty() || password.size() < 6) {
        return jsonError(res, 400, "VALIDATION_ERROR", "name, email, password(>=6) required", corsOrigin);
      }

      std::string pwHash = Password::hash(password);

      const char* params[3] = { name.c_str(), email.c_str(), pwHash.c_str() };
      PGresult* r = PQexecParams(db.conn(),
        "INSERT INTO users(name,email,password_hash) VALUES($1,$2,$3) RETURNING id",
        3, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        if (r) PQclear(r);
        return jsonError(res, 400, "REGISTER_FAILED", "Could not register (email may already exist)", corsOrigin);
      }

      long userId = std::atol(PQgetvalue(r, 0, 0));
      PQclear(r);

      std::string token = Jwt::signUser(userId, jwtSecret, 60 * 60 * 24);
      addCors(res, corsOrigin);
      res.set_content(json({{"token", token}}).dump(), "application/json");
    });

    // Login
    srv.Post("/auth/login", [&](const httplib::Request& req, httplib::Response& res) {
      auto body = json::parse(req.body, nullptr, false);
      if (body.is_discarded()) return jsonError(res, 400, "BAD_JSON", "Invalid JSON", corsOrigin);

      std::string email = body.value("email", "");
      std::string password = body.value("password", "");
      if (email.empty() || password.empty()) {
        return jsonError(res, 400, "VALIDATION_ERROR", "email and password required", corsOrigin);
      }

      const char* params[1] = { email.c_str() };
      PGresult* r = PQexecParams(db.conn(),
        "SELECT id, password_hash FROM users WHERE email=$1",
        1, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK || PQntuples(r) != 1) {
        if (r) PQclear(r);
        return jsonError(res, 401, "INVALID_CREDENTIALS", "Invalid email or password", corsOrigin);
      }

      long userId = std::atol(PQgetvalue(r, 0, 0));
      std::string storedHash = PQgetvalue(r, 0, 1);
      PQclear(r);

      if (!Password::verify(password, storedHash)) {
        return jsonError(res, 401, "INVALID_CREDENTIALS", "Invalid email or password", corsOrigin);
      }

      std::string token = Jwt::signUser(userId, jwtSecret, 60 * 60 * 24);
      addCors(res, corsOrigin);
      res.set_content(json({{"token", token}}).dump(), "application/json");
    });

    // Create transaction
    srv.Post("/transactions", [&](const httplib::Request& req, httplib::Response& res) {
      long userId = requireAuth(req, res, jwtSecret, corsOrigin);
      if (!userId) return;

      auto body = json::parse(req.body, nullptr, false);
      if (body.is_discarded()) return jsonError(res, 400, "BAD_JSON", "Invalid JSON", corsOrigin);

      std::string type = body.value("type", "");
      double amount = body.value("amount", 0.0);
      std::string date = body.value("date", "");
      std::string category = body.value("category", "");
      std::string title = body.value("title", "");
      std::string note = body.value("note", "");
      std::string currency = body.value("currency", "CAD");

      if ((type != "INCOME" && type != "EXPENSE") || amount <= 0 || date.empty() || category.empty() || title.empty()) {
        return jsonError(res, 400, "VALIDATION_ERROR", "type, amount>0, date, category, title required", corsOrigin);
      }

      std::string userStr = std::to_string(userId);
      std::string amtStr = std::to_string(amount);

      const char* params[8] = {
        userStr.c_str(), type.c_str(), amtStr.c_str(), currency.c_str(),
        date.c_str(), category.c_str(), title.c_str(), note.c_str()
      };

      PGresult* r = PQexecParams(db.conn(),
        "INSERT INTO transactions(user_id,type,amount,currency,tx_date,category,title,note) "
        "VALUES($1,$2,$3,$4,$5,$6,$7,$8) RETURNING id",
        8, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        if (r) PQclear(r);
        return jsonError(res, 500, "DB_ERROR", "Could not create transaction", corsOrigin);
      }

      long id = std::atol(PQgetvalue(r, 0, 0));
      PQclear(r);

      addCors(res, corsOrigin);
      res.set_content(json({{"id", id}}).dump(), "application/json");
    });

    // List transactions (owned by user)
    srv.Get("/transactions", [&](const httplib::Request& req, httplib::Response& res) {
      long userId = requireAuth(req, res, jwtSecret, corsOrigin);
      if (!userId) return;

      std::string userStr = std::to_string(userId);
      const char* params[1] = { userStr.c_str() };

      PGresult* r = PQexecParams(db.conn(),
        "SELECT id,type,amount,currency,tx_date,category,title,COALESCE(note,'') "
        "FROM transactions WHERE user_id=$1 "
        "ORDER BY tx_date DESC, id DESC LIMIT 200",
        1, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        if (r) PQclear(r);
        return jsonError(res, 500, "DB_ERROR", "Could not fetch transactions", corsOrigin);
      }

      json items = json::array();
      int n = PQntuples(r);
      for (int i = 0; i < n; i++) {
        items.push_back({
          {"id", std::atol(PQgetvalue(r,i,0))},
          {"type", PQgetvalue(r,i,1)},
          {"amount", std::stod(PQgetvalue(r,i,2))},
          {"currency", PQgetvalue(r,i,3)},
          {"date", PQgetvalue(r,i,4)},
          {"category", PQgetvalue(r,i,5)},
          {"title", PQgetvalue(r,i,6)},
          {"note", PQgetvalue(r,i,7)}
        });
      }
      PQclear(r);

      addCors(res, corsOrigin);
      res.set_content(json({{"items", items}}).dump(), "application/json");
    });

    // Summary
    srv.Get("/summary", [&](const httplib::Request& req, httplib::Response& res) {
      long userId = requireAuth(req, res, jwtSecret, corsOrigin);
      if (!userId) return;

      std::string userStr = std::to_string(userId);
      const char* params[1] = { userStr.c_str() };

      PGresult* r = PQexecParams(db.conn(),
        "SELECT "
        "COALESCE(SUM(CASE WHEN type='INCOME' THEN amount END),0) AS income, "
        "COALESCE(SUM(CASE WHEN type='EXPENSE' THEN amount END),0) AS expense "
        "FROM transactions WHERE user_id=$1",
        1, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        if (r) PQclear(r);
        return jsonError(res, 500, "DB_ERROR", "Could not fetch summary", corsOrigin);
      }

      double income = std::stod(PQgetvalue(r, 0, 0));
      double expense = std::stod(PQgetvalue(r, 0, 1));
      PQclear(r);

      addCors(res, corsOrigin);
      res.set_content(json({
        {"income", income},
        {"expense", expense},
        {"balance", income - expense}
      }).dump(), "application/json");
    });

    std::cout << "FlowFund API listening on " << host << ":" << port << "\n";
    srv.listen(host.c_str(), port);

  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
}
