#pragma once
#include "TransactionRepository.hpp"
#include "../db/Database.hpp"

class SqliteTransactionRepository : public TransactionRepository {
    Database& db;
public:
    explicit SqliteTransactionRepository(Database& db);
    void save(const Transaction& tx) override;
    std::vector<Transaction> findAll() override;
};
