#pragma once

#include <memory>
#include <string>

#include "sqlite3.h"

namespace repo::sqlite {

class SQLiteDb {
 public:
  explicit SQLiteDb(const std::string& path);
  ~SQLiteDb();

  SQLiteDb(const SQLiteDb&) = delete;
  SQLiteDb& operator=(const SQLiteDb&) = delete;

  sqlite3* handle() const;

 private:
  sqlite3* db_ = nullptr;
};

}  // namespace repo::sqlite
