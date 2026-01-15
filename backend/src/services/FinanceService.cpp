=#include "FinanceService.hpp"
#include "../models/Income.hpp"
#include "../models/Expense.hpp"

FinanceService::FinanceService(std::shared_ptr<TransactionRepository> r) : repo(r) {}

void FinanceService::addIncome(double a,const std::string& c,const std::string& d){
    repo->save(Income(a,c,d));
}
void FinanceService::addExpense(double a,const std::string& c,const std::string& d){
    repo->save(Expense(a,c,d));
}

double FinanceService::totalIncome(){
    double t=0; for(auto& x:repo->findAll())
        if(x.getType()==TransactionType::INCOME) t+=x.getAmount();
    return t;
}
double FinanceService::totalExpense(){
    double t=0; for(auto& x:repo->findAll())
        if(x.getType()==TransactionType::EXPENSE) t+=x.getAmount();
    return t;
}
double FinanceService::balance(){ return totalIncome()-totalExpense(); }
std::vector<Transaction> FinanceService::all(){ return repo->findAll(); }
