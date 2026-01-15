#pragma once
#include <sqlite3.h>
#include <string>

class Database {
    sqlite3* db;
public:
    explicit Database(const std::string& path);
    ~Database();
    sqlite3* get();
    void runMigrations(const std::string& file);
};
