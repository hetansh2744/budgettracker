#include "Db.hpp"
#include <stdexcept>

Db::Db(const std::string& connStr) {
  m_conn = PQconnectdb(connStr.c_str());
  if (!m_conn || PQstatus(m_conn) != CONNECTION_OK) {
    std::string err = m_conn ? PQerrorMessage(m_conn) : "null connection";
    if (m_conn) PQfinish(m_conn);
    m_conn = nullptr;
    throw std::runtime_error("DB connect failed: " + err);
  }
}

Db::~Db() {
  if (m_conn) PQfinish(m_conn);
}

void Db::execOrThrow(const std::string& sql) {
  PGresult* r = PQexec(m_conn, sql.c_str());
  if (!r) throw std::runtime_error("DB exec failed: null result");

  auto st = PQresultStatus(r);
  if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK) {
    std::string err = PQerrorMessage(m_conn);
    PQclear(r);
    throw std::runtime_error("DB exec failed: " + err);
  }

  PQclear(r);
}
