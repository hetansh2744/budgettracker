#include "Database.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("DB open failed");
}

Database::~Database() { sqlite3_close(db); }
sqlite3* Database::get() { return db; }

void Database::runMigrations(const std::string& file) {
    std::ifstream f(file);
    std::stringstream ss;
    ss << f.rdbuf();
    char* err = nullptr;
    sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, &err);
}
