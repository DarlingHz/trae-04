#pragma once

#include "models/Budget.h"
#include <vector>
#include <string>

namespace accounting {

class BudgetDAO {
public:
    // 设置/更新预算
    bool setBudget(const Budget& budget);

    // 根据 ID 查询预算
    bool getBudgetById(int id, Budget& budget);

    // 查询某个月的所有预算
    std::vector<Budget> getBudgetsByMonth(const std::string& month);

    // 查询某个月某个分类的预算
    bool getBudgetByMonthAndCategory(const std::string& month, int category_id, Budget& budget);

    // 删除预算
    bool deleteBudget(int id);

    // 删除某个月的所有预算
    bool deleteBudgetsByMonth(const std::string& month);
};

} // namespace accounting
