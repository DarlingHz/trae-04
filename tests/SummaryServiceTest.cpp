#include <catch2/catch_test_macros.hpp>
#include <catch2/catch.hpp>

#include "services/SummaryService.h"

// 测试月度汇总计算
TEST_CASE("Calculate Monthly Summary", "[SummaryService]") {
    SummaryService summaryService;

    // 测试2024-11月的月度汇总
    MonthlySummary summary = summaryService.getMonthlySummary("2024-11");

    // 验证汇总数据
    REQUIRE(summary.month == "2024-11");
    REQUIRE(summary.total_income >= 0);
    REQUIRE(summary.total_expense >= 0);
    REQUIRE(summary.balance == summary.total_income - summary.total_expense);

    // 验证分类汇总
    for (const auto& categorySummary : summary.per_category) {
        REQUIRE(categorySummary.category_id > 0);
        REQUIRE(!categorySummary.category_name.empty());
        REQUIRE(categorySummary.expense >= 0);
        REQUIRE(categorySummary.budget_limit >= 0);
        REQUIRE(categorySummary.exceed == (categorySummary.expense > categorySummary.budget_limit));
    }
}

// 测试趋势统计
TEST_CASE("Get Trend Summary", "[SummaryService]") {
    SummaryService summaryService;

    // 测试2024-01到2024-12的趋势统计
    std::vector<TrendData> trendData = summaryService.getTrendSummary("2024-01", "2024-12");

    // 验证趋势数据
    REQUIRE(trendData.size() == 12); // 12个月

    for (int i = 0; i < trendData.size(); i++) {
        const auto& data = trendData[i];

        // 验证月份格式
        REQUIRE(data.month.substr(0, 4) == "2024");
        int month = std::stoi(data.month.substr(5, 2));
        REQUIRE(month == i + 1);

        // 验证收入和支出
        REQUIRE(data.total_income >= 0);
        REQUIRE(data.total_expense >= 0);
    }
}

// 测试预算超支判断
TEST_CASE("Budget Exceed Judgment", "[SummaryService]") {
    SummaryService summaryService;

    // 测试2024-11月的月度汇总
    MonthlySummary summary = summaryService.getMonthlySummary("2024-11");

    // 找到餐饮分类
    for (const auto& categorySummary : summary.per_category) {
        if (categorySummary.category_name == "餐饮") {
            // 假设餐饮预算为1500元
            double budgetLimit = 1500.00;

            // 测试未超支情况
            if (categorySummary.expense <= budgetLimit) {
                REQUIRE(categorySummary.exceed == false);
            }

            // 测试超支情况
            if (categorySummary.expense > budgetLimit) {
                REQUIRE(categorySummary.exceed == true);
            }

            break;
        }
    }
}
