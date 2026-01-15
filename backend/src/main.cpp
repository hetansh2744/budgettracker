#include "httplib.h"
#include "nlohmann/json.hpp"

#include "Env.hpp"
#include "Db.hpp"
#include "Password.hpp"
#include "Jwt.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cstdlib>

using json = nlohmann::json;

static std::string readFile(const std::string& path) {
  std::ifstream f(path);
  if (!f.is_open()) return "";
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

static void jsonOk(httplib::Response& res, const json& body, const std::string& origin) {
  addCors(res, origin);
  res.status = 200;
  res.set_content(body.dump(), "application/json");
}

static void jsonError(httplib::Response& res,
                      int status,
                      const std::string& code,
                      const std::string& msg,
                      const std::string& origin) {
  addCors(res, origin);
  res.status = status;
  res.set_content(json({{"error", {{"code", code}, {"message", msg}}}}).dump(),
                  "application/json");
}

static bool parseJsonBody(const httplib::Request& req, json& out) {
  out = json::parse(req.body, nullptr, false);
  return !out.is_discarded();
}

static std::string ensureSslMode(const std::string& url) {
  if (url.find("sslmode=") != std::string::npos) return url;
  std::string out = url;
  if (out.find('?') == std::string::npos) out += "?sslmode=require";
  else out += "&sslmode=require";
  return out;
}

static std::string getDatabaseUrlOrEmpty() {
  return Env::firstOf({"DATABASE_URL", "POSTGRES_URL", "RENDER_DATABASE_URL"}, "");
}

static void clearRes(PGresult* r) {
  if (r) PQclear(r);
}

static long requireAuth(const httplib::Request& req,
                        httplib::Response& res,
                        const std::string& jwtSecret,
                        const std::string& origin) {
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

    std::string dbUrl = getDatabaseUrlOrEmpty();
    if (dbUrl.empty()) {
      std::cerr << "DATABASE_URL is required (or POSTGRES_URL / RENDER_DATABASE_URL)\n";
      return 1;
    }
    dbUrl = ensureSslMode(dbUrl);

    const std::string jwtSecret = Env::get("JWT_SECRET");
    if (jwtSecret.size() < 16) {
      std::cerr << "JWT_SECRET must be set (>=16 chars)\n";
      return 1;
    }

    const std::string corsOrigin = Env::get("CORS_ORIGIN", "*");

    Db db(dbUrl);

    std::string mig = readFile("migrations.sql");
    if (mig.empty()) mig = readFile("/app/migrations.sql");
    if (mig.empty()) {
      std::cerr << "migrations.sql not found\n";
      return 1;
    }
    db.execOrThrow(mig);

    httplib::Server srv;

    srv.Options(R"(.*)", [&](const httplib::Request&, httplib::Response& res) {
      addCors(res, corsOrigin);
      res.status = 204;
    });

    srv.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
      jsonOk(res, {{"ok", true}}, corsOrigin);
    });

    srv.Post("/auth/register", [&](const httplib::Request& req, httplib::Response& res) {
      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", corsOrigin);
      }

      std::string name = body.value("name", "");
      std::string email = body.value("email", "");
      std::string password = body.value("password", "");

      if (name.empty() || email.empty() || password.size() < 6) {
        return jsonError(res, 400, "VALIDATION_ERROR",
                         "name, email, password(>=6) required", corsOrigin);
      }

      std::string pwHash = Password::hash(password);

      const char* params[3] = { name.c_str(), email.c_str(), pwHash.c_str() };
      PGresult* r = PQexecParams(db.conn(),
        "INSERT INTO users(name,email,password_hash) VALUES($1,$2,$3) RETURNING id",
        3, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        clearRes(r);
        return jsonError(res, 400, "REGISTER_FAILED",
                         "Could not register (email may already exist)", corsOrigin);
      }

      long userId = std::atol(PQgetvalue(r, 0, 0));
      clearRes(r);

      std::string token = Jwt::signUser(userId, jwtSecret, 60 * 60 * 24);
      jsonOk(res, {{"token", token}}, corsOrigin);
    });

    srv.Post("/auth/login", [&](const httplib::Request& req, httplib::Response& res) {
      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", corsOrigin);
      }

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
        clearRes(r);
        return jsonError(res, 401, "INVALID_CREDENTIALS", "Invalid email or password", corsOrigin);
      }

      long userId = std::atol(PQgetvalue(r, 0, 0));
      std::string storedHash = PQgetvalue(r, 0, 1);
      clearRes(r);

      if (!Password::verify(password, storedHash)) {
        return jsonError(res, 401, "INVALID_CREDENTIALS", "Invalid email or password", corsOrigin);
      }

      std::string token = Jwt::signUser(userId, jwtSecret, 60 * 60 * 24);
      jsonOk(res, {{"token", token}}, corsOrigin);
    });

    srv.Post("/transactions", [&](const httplib::Request& req, httplib::Response& res) {
      long userId = requireAuth(req, res, jwtSecret, corsOrigin);
      if (!userId) return;

      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", corsOrigin);
      }

      std::string type = body.value("type", "");
      double amount = body.value("amount", 0.0);
      std::string date = body.value("date", "");
      std::string category = body.value("category", "");
      std::string title = body.value("title", "");
      std::string note = body.value("note", "");
      std::string currency = body.value("currency", "CAD");

      if ((type != "INCOME" && type != "EXPENSE") ||
          amount <= 0 ||
          date.empty() ||
          category.empty() ||
          title.empty()) {
        return jsonError(res, 400, "VALIDATION_ERROR",
                         "type(INCOME/EXPENSE), amount>0, date, category, title required",
                         corsOrigin);
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
        clearRes(r);
        return jsonError(res, 500, "DB_ERROR", "Could not create transaction", corsOrigin);
      }

      long id = std::atol(PQgetvalue(r, 0, 0));
      clearRes(r);

      jsonOk(res, {{"id", id}}, corsOrigin);
    });

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
        clearRes(r);
        return jsonError(res, 500, "DB_ERROR", "Could not fetch transactions", corsOrigin);
      }

      json items = json::array();
      int n = PQntuples(r);
      for (int i = 0; i < n; i++) {
        items.push_back({
          {"id", std::atol(PQgetvalue(r, i, 0))},
          {"type", PQgetvalue(r, i, 1)},
          {"amount", std::stod(PQgetvalue(r, i, 2))},
          {"currency", PQgetvalue(r, i, 3)},
          {"date", PQgetvalue(r, i, 4)},
          {"category", PQgetvalue(r, i, 5)},
          {"title", PQgetvalue(r, i, 6)},
          {"note", PQgetvalue(r, i, 7)}
        });
      }
      clearRes(r);

      jsonOk(res, {{"items", items}}, corsOrigin);
    });

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
        clearRes(r);
        return jsonError(res, 500, "DB_ERROR", "Could not fetch summary", corsOrigin);
      }

      double income = std::stod(PQgetvalue(r, 0, 0));
      double expense = std::stod(PQgetvalue(r, 0, 1));
      clearRes(r);

      jsonOk(res, {
        {"income", income},
        {"expense", expense},
        {"balance", income - expense}
      }, corsOrigin);
    });

    std::cout << "FlowFund API listening on " << host << ":" << port << "\n";
    std::cout << "CORS_ORIGIN=" << corsOrigin << "\n";
    srv.listen(host.c_str(), port);

  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
}
