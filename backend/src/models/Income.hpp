#pragma once
#include "Transaction.hpp"

class Income : public Transaction {
public:
    Income(double amount, const std::string& currency, const std::string& date)
        : Transaction(0, amount, currency, date, TransactionType::INCOME) {}
};
