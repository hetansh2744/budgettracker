#include "Db.hpp"

Db::Db(const std::string& connStr) {
  conn_ = PQconnectdb(connStr.c_str());
  if (!conn_ || PQstatus(conn_) != CONNECTION_OK) {
    std::string err = conn_ ? PQerrorMessage(conn_) : "Unknown connection error";
    if (conn_) PQfinish(conn_);
    conn_ = nullptr;
    throw std::runtime_error("DB connect failed: " + err);
  }
}

Db::~Db() {
  if (conn_) PQfinish(conn_);
}

void Db::execOrThrow(const std::string& sql) {
  PGresult* r = PQexec(conn_, sql.c_str());
  if (!r) throw std::runtime_error("DB exec failed (null result)");
  auto st = PQresultStatus(r);
  if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK) {
    std::string err = PQresultErrorMessage(r);
    PQclear(r);
    throw std::runtime_error("DB exec failed: " + err);
  }
  PQclear(r);
}
