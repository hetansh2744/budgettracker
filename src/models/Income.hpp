#pragma once
#include "Transaction.hpp"

class Income : public Transaction {
public:
    Income(int id,
           double amount,
           const std::string& currency,
           const std::string& date)
        : Transaction(id, amount, currency, date, TransactionType::INCOME) {}
};
