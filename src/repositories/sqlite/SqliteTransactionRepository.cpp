#include "SqliteTransactionRepository.hpp"
#include <sqlite3.h>

SqliteTransactionRepository::SqliteTransactionRepository(Database& db)
    : database(db) {}

void SqliteTransactionRepository::save(const Transaction& tx) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO transactions(amount, currency, date, type) VALUES(?,?,?,?)";

    sqlite3_prepare_v2(database.get(), sql, -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, tx.getAmount());
    sqlite3_bind_text(stmt, 2, tx.getCurrency().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, tx.getDate().c_str(), -1, SQLITE_TRANSIENT);

    std::string type =
        tx.getType() == TransactionType::INCOME ? "INCOME" : "EXPENSE";
    sqlite3_bind_text(stmt, 4, type.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Transaction> SqliteTransactionRepository::findAll() {
    std::vector<Transaction> result;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT id, amount, currency, date, type FROM transactions";
    sqlite3_prepare_v2(database.get(), sql, -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TransactionType type =
            std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))) == "INCOME"
                ? TransactionType::INCOME
                : TransactionType::EXPENSE;

        result.emplace_back(
            sqlite3_column_int(stmt, 0),
            sqlite3_column_double(stmt, 1),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            type
        );
    }

    sqlite3_finalize(stmt);
    return result;
}
