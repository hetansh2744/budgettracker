#pragma once
#include <memory>
#include "../repositories/TransactionRepository.hpp"

class FinanceService {
public:
    explicit FinanceService(std::shared_ptr<TransactionRepository> repo);

    void addIncome(double amount, const std::string& currency, const std::string& date);
    void addExpense(double amount, const std::string& currency, const std::string& date);

    double getTotalIncome();
    double getTotalExpense();
    double getBalance();

private:
    std::shared_ptr<TransactionRepository> repository;
};
