#include "Transaction.hpp"

Transaction::Transaction(int id,
                         double amount,
                         const std::string& currency,
                         const std::string& date,
                         TransactionType type)
    : id(id), amount(amount), currency(currency), date(date), type(type) {}

int Transaction::getId() const { return id; }
double Transaction::getAmount() const { return amount; }
std::string Transaction::getCurrency() const { return currency; }
std::string Transaction::getDate() const { return date; }
TransactionType Transaction::getType() const { return type; }
