#pragma once

#include "dao/TransactionDAO.h"
#include "dao/AccountDAO.h"
#include "dao/CategoryDAO.h"
#include "models/Transaction.h"
#include <vector>
#include <string>

namespace accounting {

class TransactionService {
public:
    TransactionService();
    ~TransactionService() = default;

    // 创建交易记录
    bool createTransaction(int account_id, int category_id, const std::string& type,
                           double amount, const std::string& time, const std::string& note,
                           Transaction& new_transaction);

    // 根据 ID 查询交易记录
    bool getTransactionById(int id, Transaction& transaction);

    // 分页查询交易记录
    TransactionPage getTransactionsByPage(const TransactionFilter& filter,
                                            int page = 1, int page_size = 10);

    // 更新交易记录
    bool updateTransaction(int id, int account_id, int category_id, const std::string& type,
                           double amount, const std::string& time, const std::string& note);

    // 删除交易记录
    bool deleteTransaction(int id);

    // 获取某个月的收入和支出总额
    bool getMonthlySummary(const std::string& month, double& total_income, double& total_expense);

    // 获取某个月每个分类的支出总额
    std::vector<std::pair<int, double>> getMonthlyExpenseByCategory(const std::string& month);

private:
    TransactionDAO transaction_dao_;
    AccountDAO account_dao_;
    CategoryDAO category_dao_;

    // 验证交易数据
    bool validateTransactionData(int account_id, int category_id, const std::string& type,
                                 double amount, const std::string& time);

    // 验证时间格式是否为 ISO8601
    bool validateISO8601Time(const std::string& time);
};

} // namespace accounting
