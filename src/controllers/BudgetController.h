#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "services/BudgetService.h"

using namespace boost::beast;
using namespace boost::beast::http;
using json = nlohmann::json;

class BudgetController {
public:
    BudgetController();
    ~BudgetController();

    response<string_body> setBudget(request<string_body>& req);
    response<string_body> getBudgets(request<string_body>& req);

private:
    accounting::BudgetService* budgetService;
};
