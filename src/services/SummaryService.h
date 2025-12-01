#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

#include "dao/TransactionDAO.h"
#include "dao/BudgetDAO.h"
#include "dao/CategoryDAO.h"

namespace accounting {

// 分类汇总数据结构
struct CategorySummary {
    int category_id;
    std::string category_name;
    double expense;
    double budget_limit;
    bool exceed;
};

// 月度汇总数据结构
struct MonthlySummary {
    std::string month;
    double total_income;
    double total_expense;
    double balance;
    std::vector<CategorySummary> per_category;
};

// 趋势统计数据结构
struct TrendData {
    std::string month;
    double total_income;
    double total_expense;
};

class SummaryService {
public:
    SummaryService();
    ~SummaryService();

    MonthlySummary getMonthlySummary(const std::string& month);
    std::vector<TrendData> getTrendSummary(const std::string& from, const std::string& to);

private:
    TransactionDAO* transactionDAO;
    BudgetDAO* budgetDAO;
    CategoryDAO* categoryDAO;

    // 月度汇总缓存
    std::unordered_map<std::string, MonthlySummary> monthlySummaryCache;
    std::mutex cacheMutex;

    void validateMonthFormat(const std::string& month);
    void validateDateRange(const std::string& from, const std::string& to);
    MonthlySummary calculateMonthlySummary(const std::string& month);
};

} // namespace accounting
