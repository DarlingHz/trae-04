#include "SummaryService.h"

#include <regex>
#include <stdexcept>
#include <algorithm>

using namespace accounting;

SummaryService::SummaryService() {
    transactionDAO = new TransactionDAO();
    budgetDAO = new BudgetDAO();
    categoryDAO = new CategoryDAO();
}

SummaryService::~SummaryService() {
    delete transactionDAO;
    delete budgetDAO;
    delete categoryDAO;
}

MonthlySummary SummaryService::getMonthlySummary(const std::string& month) {
    // 验证月份格式
    validateMonthFormat(month);

    // 检查缓存中是否存在
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = monthlySummaryCache.find(month);
    if (it != monthlySummaryCache.end()) {
        return it->second;
    }

    // 计算月度汇总
    MonthlySummary summary = calculateMonthlySummary(month);

    // 将结果存入缓存
    monthlySummaryCache[month] = summary;

    return summary;
}

std::vector<TrendData> SummaryService::getTrendSummary(const std::string& from, const std::string& to) {
    // 验证日期范围
    validateDateRange(from, to);

    // 获取从from到to的所有月份
    std::vector<std::string> months;
    int fromYear = std::stoi(from.substr(0, 4));
    int fromMonth = std::stoi(from.substr(5, 2));
    int toYear = std::stoi(to.substr(0, 4));
    int toMonth = std::stoi(to.substr(5, 2));

    for (int year = fromYear; year <= toYear; year++) {
        int startMonth = (year == fromYear) ? fromMonth : 1;
        int endMonth = (year == toYear) ? toMonth : 12;

        for (int month = startMonth; month <= endMonth; month++) {
            std::string monthStr = std::to_string(year) + "-" + (month < 10 ? "0" : "") + std::to_string(month);
            months.push_back(monthStr);
        }
    }

    // 计算每个月的趋势数据
    std::vector<TrendData> trendData;
    for (const auto& month : months) {
        TrendData data;
        data.month = month;

        // 获取该月的收入和支出
        double total_income = 0.0;
        double total_expense = 0.0;
        transactionDAO->getMonthlySummary(month, total_income, total_expense);
        data.total_income = total_income;
        data.total_expense = total_expense;

        trendData.push_back(data);
    }

    return trendData;
}

void SummaryService::validateMonthFormat(const std::string& month) {
    // 验证月份格式是否为YYYY-MM
    std::regex monthRegex("\\d{4}-\\d{2}");
    if (!std::regex_match(month, monthRegex)) {
        throw std::invalid_argument("Invalid month format. Expected YYYY-MM");
    }
}

void SummaryService::validateDateRange(const std::string& from, const std::string& to) {
    // 验证起始和结束月份格式
    validateMonthFormat(from);
    validateMonthFormat(to);

    // 验证起始月份是否早于或等于结束月份
    int fromYear = std::stoi(from.substr(0, 4));
    int fromMonth = std::stoi(from.substr(5, 2));
    int toYear = std::stoi(to.substr(0, 4));
    int toMonth = std::stoi(to.substr(5, 2));

    if (fromYear > toYear || (fromYear == toYear && fromMonth > toMonth)) {
        throw std::invalid_argument("Start month must be earlier than or equal to end month");
    }
}

MonthlySummary SummaryService::calculateMonthlySummary(const std::string& month) {
    MonthlySummary summary;
    summary.month = month;

    // 获取该月的收入和支出
    double total_income = 0.0;
    double total_expense = 0.0;
    transactionDAO->getMonthlySummary(month, total_income, total_expense);
    summary.total_income = total_income;
    summary.total_expense = total_expense;
    summary.balance = total_income - total_expense;

    // 获取该月的所有预算
    std::vector<Budget> budgets = budgetDAO->getBudgetsByMonth(month);

    // 获取该月的支出分类汇总
    std::vector<std::pair<int, double>> expenseByCategory = transactionDAO->getMonthlyExpenseByCategory(month);

    // 合并预算和支出数据
    for (const auto& expenseItem : expenseByCategory) {
        CategorySummary categorySummary;
        categorySummary.category_id = expenseItem.first;
        categorySummary.expense = expenseItem.second;

        // 查找该分类的预算
        auto budgetIt = std::find_if(budgets.begin(), budgets.end(),
            [&](const Budget& budget) { return budget.getCategoryId() == expenseItem.first; });

        if (budgetIt != budgets.end()) {
            categorySummary.budget_limit = budgetIt->getLimit();
            categorySummary.exceed = (expenseItem.second > budgetIt->getLimit());
        } else {
            categorySummary.budget_limit = 0.0;
            categorySummary.exceed = false;
        }

        // 获取分类名称
        Category category;
        if (categoryDAO->getCategoryById(expenseItem.first, category)) {
            categorySummary.category_name = category.getName();
        } else {
            categorySummary.category_name = "Unknown Category";
        }

        summary.per_category.push_back(categorySummary);
    }

    return summary;
}