#pragma once
#include <string>
#include <libpq-fe.h>

class Db {
public:
  explicit Db(const std::string& connStr);
  ~Db();

  Db(const Db&) = delete;
  Db& operator=(const Db&) = delete;

  PGconn* conn() const { return m_conn; }
  void execOrThrow(const std::string& sql);

private:
  PGconn* m_conn = nullptr;
};
