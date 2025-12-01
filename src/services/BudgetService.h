#pragma once

#include <vector>

#include "dao/BudgetDAO.h"
#include "models/Budget.h"

namespace accounting {

// 预算项目结构体
struct BudgetItem {
    int category_id;
    double limit;
};

class BudgetService {
public:
    BudgetService();
    ~BudgetService();

    void setBudget(const std::string& month, const std::vector<BudgetItem>& items);
    std::vector<Budget> getBudgetsByMonth(const std::string& month);
    Budget getBudgetByMonthAndCategory(const std::string& month, int category_id);

private:
    BudgetDAO* budgetDAO;
    void validateBudgetData(const std::string& month, const std::vector<BudgetItem>& items);
    void validateMonthFormat(const std::string& month);
};

} // namespace accounting
