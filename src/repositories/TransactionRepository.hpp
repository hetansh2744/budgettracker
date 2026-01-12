#pragma once
#include <vector>
#include "../models/Transaction.hpp"

class TransactionRepository {
public:
    virtual ~TransactionRepository() = default;

    virtual void save(const Transaction& tx) = 0;
    virtual std::vector<Transaction> findAll() = 0;
};
