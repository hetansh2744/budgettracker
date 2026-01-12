#include "SqliteIssueRepository.hpp"

SqliteIssueRepository::SqliteIssueRepository(Database& db)
    : database(db) {}

void SqliteIssueRepository::addIssue(const Issue& issue) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO issues(title, description, priority, status) VALUES(?,?,?,?)";

    sqlite3_prepare_v2(database.get(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, issue.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, issue.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, "HIGH", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, "OPEN", -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Issue> SqliteIssueRepository::findAll() {
    std::vector<Issue> issues;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT id, title, description FROM issues";
    sqlite3_prepare_v2(database.get(), sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        issues.emplace_back(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
        );
    }

    sqlite3_finalize(stmt);
    return issues;
}
