#include "SqliteTransactionRepository.hpp"

SqliteTransactionRepository::SqliteTransactionRepository(Database& db) : db(db) {}

void SqliteTransactionRepository::save(const Transaction& tx) {
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO transactions(amount,currency,date,type) VALUES(?,?,?,?)";
    sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, tx.getAmount());
    sqlite3_bind_text(stmt, 2, tx.getCurrency().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, tx.getDate().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4,
        tx.getType() == TransactionType::INCOME ? "INCOME" : "EXPENSE",
        -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Transaction> SqliteTransactionRepository::findAll() {
    std::vector<Transaction> list;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.get(),
        "SELECT id,amount,currency,date,type FROM transactions",
        -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        list.emplace_back(
            sqlite3_column_int(stmt,0),
            sqlite3_column_double(stmt,1),
            (const char*)sqlite3_column_text(stmt,2),
            (const char*)sqlite3_column_text(stmt,3),
            std::string((const char*)sqlite3_column_text(stmt,4)) == "INCOME"
                ? TransactionType::INCOME
                : TransactionType::EXPENSE
        );
    }
    sqlite3_finalize(stmt);
    return list;
}
