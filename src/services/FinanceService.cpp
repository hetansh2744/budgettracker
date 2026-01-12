#include "FinanceService.hpp"
#include "../models/Income.hpp"
#include "../models/Expense.hpp"

FinanceService::FinanceService(std::shared_ptr<TransactionRepository> repo)
    : repository(repo) {}

void FinanceService::addIncome(double amount,
                               const std::string& currency,
                               const std::string& date) {
    Income income(0, amount, currency, date);
    repository->save(income);
}

void FinanceService::addExpense(double amount,
                                const std::string& currency,
                                const std::string& date) {
    Expense expense(0, amount, currency, date);
    repository->save(expense);
}

double FinanceService::getTotalIncome() {
    double total = 0;
    for (const auto& tx : repository->findAll()) {
        if (tx.getType() == TransactionType::INCOME)
            total += tx.getAmount();
    }
    return total;
}

double FinanceService::getTotalExpense() {
    double total = 0;
    for (const auto& tx : repository->findAll()) {
        if (tx.getType() == TransactionType::EXPENSE)
            total += tx.getAmount();
    }
    return total;
}

double FinanceService::getBalance() {
    return getTotalIncome() - getTotalExpense();
}
