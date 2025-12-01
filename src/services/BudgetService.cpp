#include "BudgetService.h"

#include <regex>
#include <stdexcept>

using namespace accounting;

BudgetService::BudgetService() {
    budgetDAO = new BudgetDAO();
}

BudgetService::~BudgetService() {
    delete budgetDAO;
}

void BudgetService::setBudget(const std::string& month, const std::vector<BudgetItem>& items) {
    // 验证预算数据
    validateBudgetData(month, items);

    // 设置预算
    for (const auto& item : items) {
        Budget budget;
        budget.setMonth(month);
        budget.setCategoryId(item.category_id);
        budget.setLimit(item.limit);
        budgetDAO->setBudget(budget);
    }
}

std::vector<Budget> BudgetService::getBudgetsByMonth(const std::string& month) {
    // 验证月份格式
    validateMonthFormat(month);

    // 获取该月的所有预算
    return budgetDAO->getBudgetsByMonth(month);
}

Budget BudgetService::getBudgetByMonthAndCategory(const std::string& month, int category_id) {
    // 验证月份格式
    validateMonthFormat(month);

    // 验证分类ID
    if (category_id <= 0) {
        throw std::invalid_argument("Invalid category ID");
    }

    // 获取该月该分类的预算
    Budget budget;
    if (!budgetDAO->getBudgetByMonthAndCategory(month, category_id, budget)) {
        throw std::runtime_error("Budget not found");
    }
    return budget;
}

void BudgetService::validateBudgetData(const std::string& month, const std::vector<BudgetItem>& items) {
    // 验证月份格式
    validateMonthFormat(month);

    // 验证预算项目
    if (items.empty()) {
        throw std::invalid_argument("No budget items provided");
    }

    for (const auto& item : items) {
        // 验证分类ID
        if (item.category_id <= 0) {
            throw std::invalid_argument("Invalid category ID in budget item");
        }

        // 验证预算金额
        if (item.limit < 0) {
            throw std::invalid_argument("Budget limit cannot be negative");
        }
    }
}

void BudgetService::validateMonthFormat(const std::string& month) {
    // 验证月份格式是否为YYYY-MM
    std::regex monthRegex("\\d{4}-\\d{2}");
    if (!std::regex_match(month, monthRegex)) {
        throw std::invalid_argument("Invalid month format. Expected YYYY-MM");
    }
}