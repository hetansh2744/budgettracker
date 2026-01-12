#pragma once
#include "Transaction.hpp"

class Expense : public Transaction {
public:
    Expense(int id,
            double amount,
            const std::string& currency,
            const std::string& date)
        : Transaction(id, amount, currency, date, TransactionType::EXPENSE) {}
};
