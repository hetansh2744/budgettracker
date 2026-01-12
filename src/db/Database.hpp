#pragma once
#include <sqlite3.h>
#include <string>

class Database {
public:
    explicit Database(const std::string& dbPath);
    ~Database();

    sqlite3* get();
    void runMigrations(const std::string& migrationFile);

private:
    sqlite3* db;
};
