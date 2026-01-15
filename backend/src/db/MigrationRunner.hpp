#pragma once

#include <string>
#include <vector>

namespace db {

struct Migration {
  std::string id;
  std::string filename;
};

class MigrationRunner {
 public:
  explicit MigrationRunner(const std::string& sqlitePath);

  void runAll(const std::string& migrationsDir,
              const std::vector<Migration>& orderedMigrations);

 private:
  std::string sqlitePath_;
};

}  // namespace db
