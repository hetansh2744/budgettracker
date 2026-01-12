#pragma once
#include <string>

enum class TransactionType {
    INCOME,
    EXPENSE
};

class Transaction {
protected:
    int id;
    double amount;
    std::string currency;
    std::string date;
    TransactionType type;

public:
    Transaction(int id,
                double amount,
                const std::string& currency,
                const std::string& date,
                TransactionType type);

    virtual ~Transaction() = default;

    int getId() const;
    double getAmount() const;
    std::string getCurrency() const;
    std::string getDate() const;
    TransactionType getType() const;
};
