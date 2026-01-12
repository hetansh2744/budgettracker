#include "Database.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

Database::Database(const std::string& dbPath) : db(nullptr) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }
}

Database::~Database() {
    sqlite3_close(db);
}

sqlite3* Database::get() {
    return db;
}

void Database::runMigrations(const std::string& migrationFile) {
    std::ifstream file(migrationFile);
    std::stringstream buffer;
    buffer << file.rdbuf();

    char* errorMsg = nullptr;
    if (sqlite3_exec(db, buffer.str().c_str(), nullptr, nullptr, &errorMsg) != SQLITE_OK) {
        std::string error = errorMsg;
        sqlite3_free(errorMsg);
        throw std::runtime_error(error);
    }
}
