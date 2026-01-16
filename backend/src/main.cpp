#include "httplib.h"
#include "nlohmann/json.hpp"

#include "Db.hpp"
#include "Env.hpp"
#include "Jwt.hpp"
#include "Password.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <libpq-fe.h>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

// ---------------------- Utilities ----------------------

static std::string readFile(const std::string& path) {
  std::ifstream f(path);
  if (!f.is_open()) return "";
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

static void clearRes(PGresult* r) {
  if (r) PQclear(r);
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
  return Env::firstOf({"DATABASE_URL", "POSTGRES_URL", "RENDER_DATABASE_URL"},
                      "");
}

static std::string trimCopy(std::string s) {
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

static std::vector<std::string> splitCsv(const std::string& s) {
  std::vector<std::string> out;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, ',')) {
    item = trimCopy(item);
    if (!item.empty()) out.push_back(item);
  }
  return out;
}

static std::string getHeaderOrEmpty(const httplib::Request& req,
                                    const std::string& key) {
  auto it = req.headers.find(key);
  if (it == req.headers.end()) return "";
  return it->second;
}

// ---------------------- CORS ----------------------
//
// CORS_ORIGIN can be:
//   - "*" (allow any origin; we echo request Origin when present for best browser compatibility)
//   - a single origin: "https://hetansh2744.github.io"
//   - multiple comma-separated: "https://a.com,https://b.com"
//
// IMPORTANT: We do NOT use cookies here, so we do not set Allow-Credentials.

static std::string resolveCorsOrigin(const httplib::Request& req,
                                    const std::string& corsOriginEnv) {
  const std::string reqOrigin = getHeaderOrEmpty(req, "Origin");
  if (corsOriginEnv.empty()) return "";  // disable CORS

  // If env is "*", best practice is to echo request Origin if present.
  if (corsOriginEnv == "*") {
    if (!reqOrigin.empty()) return reqOrigin;
    return "*";
  }

  // If env is a list, allow if exact match.
  const auto allowed = splitCsv(corsOriginEnv);
  if (allowed.empty()) return "";

  for (const auto& a : allowed) {
    if (a == reqOrigin) return reqOrigin;
  }

  // Not allowed
  return "";
}

static void addCors(httplib::Response& res, const std::string& origin) {
  if (origin.empty()) return;

  res.set_header("Access-Control-Allow-Origin", origin.c_str());
  res.set_header("Vary", "Origin");  // important when echoing dynamic origin

  res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
  res.set_header("Access-Control-Allow-Methods",
                 "GET, POST, PUT, PATCH, DELETE, OPTIONS");
  res.set_header("Access-Control-Max-Age", "86400");
}

static void jsonOk(httplib::Response& res, const json& body,
                   const std::string& origin) {
  addCors(res, origin);
  res.status = 200;
  res.set_content(body.dump(), "application/json");
}

static void jsonError(httplib::Response& res, int status,
                      const std::string& code, const std::string& msg,
                      const std::string& origin) {
  addCors(res, origin);
  res.status = status;
  res.set_content(json({{"error", {{"code", code}, {"message", msg}}}}).dump(),
                  "application/json");
}

// ---------------------- Auth ----------------------

static long requireAuth(const httplib::Request& req, httplib::Response& res,
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

// ---------------------- Main ----------------------

int main() {
  try {
    const int port = Env::getInt("PORT", 10000);
    const std::string host = "0.0.0.0";

    std::string dbUrl = getDatabaseUrlOrEmpty();
    if (dbUrl.empty()) {
      std::cerr
          << "DATABASE_URL is required (or POSTGRES_URL / RENDER_DATABASE_URL)\n";
      return 1;
    }
    dbUrl = ensureSslMode(dbUrl);

    const std::string jwtSecret = Env::get("JWT_SECRET");
    if (jwtSecret.size() < 16) {
      std::cerr << "JWT_SECRET must be set (>=16 chars)\n";
      return 1;
    }

    // Set this on Render:
    // CORS_ORIGIN=https://hetansh2744.github.io
    // or multiple:
    // CORS_ORIGIN=https://hetansh2744.github.io,https://your-static.onrender.com
    const std::string corsOriginEnv =
        Env::get("CORS_ORIGIN", "https://hetansh2744.github.io");

    Db db(dbUrl);

    // Run migrations (local then /app)
    std::string mig = readFile("migrations.sql");
    if (mig.empty()) mig = readFile("/app/migrations.sql");
    if (mig.empty()) {
      std::cerr
          << "migrations.sql not found (expected ./migrations.sql or "
             "/app/migrations.sql)\n";
      return 1;
    }
    db.execOrThrow(mig);

    httplib::Server srv;

    // Preflight (CORS)
    srv.Options(R"(.*)", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);
      addCors(res, origin);
      res.status = 204;
    });

    // Root
    srv.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);
      addCors(res, origin);
      res.set_content("FlowFund API is running. Try /health", "text/plain");
    });

    // Health
    srv.Get("/health", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);
      jsonOk(res, {{"ok", true}}, origin);
    });

    // Register
    srv.Post("/auth/register", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", origin);
      }

      std::string name = body.value("name", "");
      std::string email = body.value("email", "");
      std::string password = body.value("password", "");

      if (name.empty() || email.empty() || password.size() < 6) {
        return jsonError(res, 400, "VALIDATION_ERROR",
                         "name, email, password(>=6) required", origin);
      }

      std::string pwHash = Password::hash(password);

      const char* params[3] = {name.c_str(), email.c_str(), pwHash.c_str()};
      PGresult* r = PQexecParams(
          db.conn(),
          "INSERT INTO users(name,email,password_hash) "
          "VALUES($1,$2,$3) RETURNING id",
          3, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        clearRes(r);
        return jsonError(res, 400, "REGISTER_FAILED",
                         "Could not register (email may already exist)", origin);
      }

      long userId = std::atol(PQgetvalue(r, 0, 0));
      clearRes(r);

      std::string token = Jwt::signUser(userId, jwtSecret, 60 * 60 * 24);
      jsonOk(res, {{"token", token}}, origin);
    });

    // Login
    srv.Post("/auth/login", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", origin);
      }

      std::string email = body.value("email", "");
      std::string password = body.value("password", "");
      if (email.empty() || password.empty()) {
        return jsonError(res, 400, "VALIDATION_ERROR",
                         "email and password required", origin);
      }

      const char* params[1] = {email.c_str()};
      PGresult* r = PQexecParams(
          db.conn(),
          "SELECT id, password_hash FROM users WHERE email=$1",
          1, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK || PQntuples(r) != 1) {
        clearRes(r);
        return jsonError(res, 401, "INVALID_CREDENTIALS",
                         "Invalid email or password", origin);
      }

      long userId = std::atol(PQgetvalue(r, 0, 0));
      std::string storedHash = PQgetvalue(r, 0, 1);
      clearRes(r);

      if (!Password::verify(password, storedHash)) {
        return jsonError(res, 401, "INVALID_CREDENTIALS",
                         "Invalid email or password", origin);
      }

      std::string token = Jwt::signUser(userId, jwtSecret, 60 * 60 * 24);
      jsonOk(res, {{"token", token}}, origin);
    });

    // Create transaction
    srv.Post("/transactions", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      long userId = requireAuth(req, res, jwtSecret, origin);
      if (!userId) return;

      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", origin);
      }

      std::string type = body.value("type", "");
      double amount = body.value("amount", 0.0);
      std::string date = body.value("date", "");
      std::string category = body.value("category", "");
      std::string title = body.value("title", "");
      std::string note = body.value("note", "");
      std::string currency = body.value("currency", "CAD");

      if ((type != "INCOME" && type != "EXPENSE") || amount <= 0 || date.empty() ||
          category.empty() || title.empty()) {
        return jsonError(res, 400, "VALIDATION_ERROR",
                         "type(INCOME/EXPENSE), amount>0, date, category, title required",
                         origin);
      }

      std::string userStr = std::to_string(userId);
      std::string amtStr = std::to_string(amount);

      const char* params[8] = {
          userStr.c_str(), type.c_str(),      amtStr.c_str(),
          currency.c_str(), date.c_str(),     category.c_str(),
          title.c_str(),     note.c_str(),
      };

      PGresult* r = PQexecParams(
          db.conn(),
          "INSERT INTO transactions(user_id,type,amount,currency,tx_date,category,title,note) "
          "VALUES($1,$2,$3,$4,$5,$6,$7,$8) RETURNING id",
          8, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        clearRes(r);
        return jsonError(res, 500, "DB_ERROR", "Could not create transaction", origin);
      }

      long id = std::atol(PQgetvalue(r, 0, 0));
      clearRes(r);

      jsonOk(res, {{"id", id}}, origin);
    });

    // List transactions
    srv.Get("/transactions", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      long userId = requireAuth(req, res, jwtSecret, origin);
      if (!userId) return;

      std::string userStr = std::to_string(userId);
      const char* params[1] = {userStr.c_str()};

      PGresult* r = PQexecParams(
          db.conn(),
          "SELECT id,type,amount,currency,tx_date,category,title,COALESCE(note,'') "
          "FROM transactions WHERE user_id=$1 "
          "ORDER BY tx_date DESC, id DESC LIMIT 200",
          1, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        clearRes(r);
        return jsonError(res, 500, "DB_ERROR", "Could not fetch transactions", origin);
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
            {"note", PQgetvalue(r, i, 7)},
        });
      }
      clearRes(r);

      jsonOk(res, {{"items", items}}, origin);
    });

    // EDIT transaction (PUT) - full update
    srv.Put(R"(/transactions/(\d+))",
            [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      long userId = requireAuth(req, res, jwtSecret, origin);
      if (!userId) return;

      const long txId = std::atol(req.matches[1].str().c_str());

      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", origin);
      }

      std::string type = body.value("type", "");
      double amount = body.value("amount", 0.0);
      std::string date = body.value("date", "");
      std::string category = body.value("category", "");
      std::string title = body.value("title", "");
      std::string note = body.value("note", "");
      std::string currency = body.value("currency", "CAD");

      if ((type != "INCOME" && type != "EXPENSE") || amount <= 0 || date.empty() ||
          category.empty() || title.empty()) {
        return jsonError(res, 400, "VALIDATION_ERROR",
                         "type(INCOME/EXPENSE), amount>0, date, category, title required",
                         origin);
      }

      std::string userStr = std::to_string(userId);
      std::string txStr = std::to_string(txId);
      std::string amtStr = std::to_string(amount);

      const char* params[9] = {
          type.c_str(), amtStr.c_str(), currency.c_str(),
          date.c_str(), category.c_str(), title.c_str(), note.c_str(),
          txStr.c_str(), userStr.c_str()
      };

      PGresult* r = PQexecParams(
          db.conn(),
          "UPDATE transactions SET type=$1, amount=$2, currency=$3, tx_date=$4, "
          "category=$5, title=$6, note=$7 "
          "WHERE id=$8 AND user_id=$9 RETURNING id",
          9, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK || PQntuples(r) != 1) {
        clearRes(r);
        return jsonError(res, 404, "NOT_FOUND", "Transaction not found", origin);
      }

      clearRes(r);
      jsonOk(res, {{"ok", true}}, origin);
    });

    // EDIT transaction (PATCH) - partial update
    srv.Patch(R"(/transactions/(\d+))",
              [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      long userId = requireAuth(req, res, jwtSecret, origin);
      if (!userId) return;

      const long txId = std::atol(req.matches[1].str().c_str());

      json body;
      if (!parseJsonBody(req, body)) {
        return jsonError(res, 400, "BAD_JSON", "Invalid JSON", origin);
      }

      // Fetch existing first
      std::string userStr = std::to_string(userId);
      std::string txStr = std::to_string(txId);
      const char* paramsSel[2] = {txStr.c_str(), userStr.c_str()};

      PGresult* sel = PQexecParams(
          db.conn(),
          "SELECT type,amount,currency,tx_date,category,title,COALESCE(note,'') "
          "FROM transactions WHERE id=$1 AND user_id=$2",
          2, nullptr, paramsSel, nullptr, nullptr, 0);

      if (!sel || PQresultStatus(sel) != PGRES_TUPLES_OK || PQntuples(sel) != 1) {
        clearRes(sel);
        return jsonError(res, 404, "NOT_FOUND", "Transaction not found", origin);
      }

      std::string type = PQgetvalue(sel, 0, 0);
      double amount = std::stod(PQgetvalue(sel, 0, 1));
      std::string currency = PQgetvalue(sel, 0, 2);
      std::string date = PQgetvalue(sel, 0, 3);
      std::string category = PQgetvalue(sel, 0, 4);
      std::string title = PQgetvalue(sel, 0, 5);
      std::string note = PQgetvalue(sel, 0, 6);
      clearRes(sel);

      // Apply patches
      if (body.contains("type")) type = body.value("type", type);
      if (body.contains("amount")) amount = body.value("amount", amount);
      if (body.contains("currency")) currency = body.value("currency", currency);
      if (body.contains("date")) date = body.value("date", date);
      if (body.contains("category")) category = body.value("category", category);
      if (body.contains("title")) title = body.value("title", title);
      if (body.contains("note")) note = body.value("note", note);

      if ((type != "INCOME" && type != "EXPENSE") || amount <= 0 || date.empty() ||
          category.empty() || title.empty()) {
        return jsonError(res, 400, "VALIDATION_ERROR",
                         "type(INCOME/EXPENSE), amount>0, date, category, title required",
                         origin);
      }

      std::string amtStr = std::to_string(amount);

      const char* paramsUpd[9] = {
          type.c_str(), amtStr.c_str(), currency.c_str(),
          date.c_str(), category.c_str(), title.c_str(), note.c_str(),
          txStr.c_str(), userStr.c_str()
      };

      PGresult* r = PQexecParams(
          db.conn(),
          "UPDATE transactions SET type=$1, amount=$2, currency=$3, tx_date=$4, "
          "category=$5, title=$6, note=$7 "
          "WHERE id=$8 AND user_id=$9 RETURNING id",
          9, nullptr, paramsUpd, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK || PQntuples(r) != 1) {
        clearRes(r);
        return jsonError(res, 404, "NOT_FOUND", "Transaction not found", origin);
      }

      clearRes(r);
      jsonOk(res, {{"ok", true}}, origin);
    });

    // DELETE transaction
    srv.Delete(R"(/transactions/(\d+))",
               [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      long userId = requireAuth(req, res, jwtSecret, origin);
      if (!userId) return;

      const long txId = std::atol(req.matches[1].str().c_str());

      std::string userStr = std::to_string(userId);
      std::string txStr = std::to_string(txId);
      const char* params[2] = {txStr.c_str(), userStr.c_str()};

      PGresult* r = PQexecParams(
          db.conn(),
          "DELETE FROM transactions WHERE id=$1 AND user_id=$2 RETURNING id",
          2, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK || PQntuples(r) != 1) {
        clearRes(r);
        return jsonError(res, 404, "NOT_FOUND", "Transaction not found", origin);
      }

      clearRes(r);
      jsonOk(res, {{"ok", true}}, origin);
    });

    // Summary
    srv.Get("/summary", [&](const httplib::Request& req, httplib::Response& res) {
      const std::string origin = resolveCorsOrigin(req, corsOriginEnv);

      long userId = requireAuth(req, res, jwtSecret, origin);
      if (!userId) return;

      std::string userStr = std::to_string(userId);
      const char* params[1] = {userStr.c_str()};

      PGresult* r = PQexecParams(
          db.conn(),
          "SELECT "
          "COALESCE(SUM(CASE WHEN type='INCOME' THEN amount END),0) AS income, "
          "COALESCE(SUM(CASE WHEN type='EXPENSE' THEN amount END),0) AS expense "
          "FROM transactions WHERE user_id=$1",
          1, nullptr, params, nullptr, nullptr, 0);

      if (!r || PQresultStatus(r) != PGRES_TUPLES_OK) {
        clearRes(r);
        return jsonError(res, 500, "DB_ERROR", "Could not fetch summary", origin);
      }

      double income = std::stod(PQgetvalue(r, 0, 0));
      double expense = std::stod(PQgetvalue(r, 0, 1));
      clearRes(r);

      jsonOk(res,
             {{"income", income},
              {"expense", expense},
              {"balance", income - expense}},
             origin);
    });

    std::cout << "FlowFund API listening on " << host << ":" << port << "\n";
    std::cout << "CORS_ORIGIN=" << corsOriginEnv << "\n";

    srv.listen(host.c_str(), port);

  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
}
