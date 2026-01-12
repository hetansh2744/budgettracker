#include "SqliteProjectRepository.hpp"

SqliteProjectRepository::SqliteProjectRepository(Database& db)
    : database(db) {}

void SqliteProjectRepository::addProject(const Project& project) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO projects(name) VALUES(?)";

    sqlite3_prepare_v2(database.get(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, project.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Project> SqliteProjectRepository::findAll() {
    std::vector<Project> projects;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT id, name FROM projects";
    sqlite3_prepare_v2(database.get(), sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        projects.emplace_back(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
        );
    }

    sqlite3_finalize(stmt);
    return projects;
}
