#pragma once
#include <string>
#include <stdexcept>
#include <libpq-fe.h>

class Db {
public:
  explicit Db(const std::string& connStr);
  ~Db();

  Db(const Db&) = delete;
  Db& operator=(const Db&) = delete;

  PGconn* conn() const { return conn_; }
  void execOrThrow(const std::string& sql);

private:
  PGconn* conn_{nullptr};
};
