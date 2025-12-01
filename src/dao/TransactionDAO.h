#pragma once

#include "models/Transaction.h"
#include <vector>
#include <string>

namespace accounting {

struct TransactionFilter {
    std::string from_time; // 起始时间
    std::string to_time;   // 结束时间
    int account_id = 0;    // 账户 ID（0 表示不限制）
    int category_id = 0;   // 分类 ID（0 表示不限制）
    std::string type;       // 类型（income/expense，空表示不限制）
    double amount_min = 0.0;// 最小金额（0 表示不限制）
    double amount_max = 0.0;// 最大金额（0 表示不限制）
};

struct TransactionPage {
    std::vector<Transaction> transactions;
    int total_count = 0;
    int page = 1;
    int page_size = 10;
};

class TransactionDAO {
public:
    // 创建交易记录
    bool createTransaction(const Transaction& transaction, int& new_transaction_id);

    // 根据 ID 查询交易记录
    bool getTransactionById(int id, Transaction& transaction);

    // 分页查询交易记录
    TransactionPage getTransactionsByPage(const TransactionFilter& filter,
                                            int page = 1, int page_size = 10);

    // 更新交易记录
    bool updateTransaction(const Transaction& transaction);

    // 删除交易记录
    bool deleteTransaction(int id);

    // 获取某个月的收入和支出总额
    bool getMonthlySummary(const std::string& month, double& total_income, double& total_expense);

    // 获取某个月每个分类的支出总额
    std::vector<std::pair<int, double>> getMonthlyExpenseByCategory(const std::string& month);
};

} // namespace accounting
