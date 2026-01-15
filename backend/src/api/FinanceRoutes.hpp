#pragma once
#include "httplib.h"
#include "json.hpp"
#include "../services/FinanceService.hpp"

using json = nlohmann::json;

inline void registerFinanceRoutes(httplib::Server& s, FinanceService& f) {

    s.Post("/income",[&](auto& r, auto& res){
        auto j=json::parse(r.body);
        f.addIncome(j["amount"],"CAD",j["date"]);
        res.set_content(R"({"ok":true})","application/json");
    });

    s.Post("/expenses",[&](auto& r, auto& res){
        auto j=json::parse(r.body);
        f.addExpense(j["amount"],"CAD",j["date"]);
        res.set_content(R"({"ok":true})","application/json");
    });

    s.Get("/summary",[&](auto&, auto& res){
        json j={{"income",f.totalIncome()},
                {"expense",f.totalExpense()},
                {"balance",f.balance()}};
        res.set_content(j.dump(),"application/json");
    });
}
