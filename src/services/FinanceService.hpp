#pragma once
#include <memory>
#include "../repositories/TransactionRepository.hpp"

class FinanceService {
    std::shared_ptr<TransactionRepository> repo;
public:
    explicit FinanceService(std::shared_ptr<TransactionRepository> repo);

    void addIncome(double amt, const std::string& cur, const std::string& date);
    void addExpense(double amt, const std::string& cur, const std::string& date);

    double totalIncome();
    double totalExpense();
    double balance();
    std::vector<Transaction> all();
};
