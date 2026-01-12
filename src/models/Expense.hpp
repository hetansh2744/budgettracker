#pragma once
#include "Transaction.hpp"

class Expense : public Transaction {
public:
    Expense(double amount, const std::string& currency, const std::string& date)
        : Transaction(0, amount, currency, date, TransactionType::EXPENSE) {}
};
