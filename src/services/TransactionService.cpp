#include "TransactionService.h"
#include <iostream>
#include <algorithm>
#include <regex>

namespace accounting {

TransactionService::TransactionService() {
}

bool TransactionService::createTransaction(int account_id, int category_id, const std::string& type,
                                             double amount, const std::string& time, const std::string& note,
                                             Transaction& new_transaction) {
    // 验证交易数据
    if (!validateTransactionData(account_id, category_id, type, amount, time)) {
        std::cerr << "Invalid transaction data." << std::endl;
        return false;
    }

    // 创建交易对象
    Transaction transaction(0, account_id, category_id, type, amount, time, note);

    // 调用 DAO 创建交易记录
    int new_transaction_id = 0;
    if (!transaction_dao_.createTransaction(transaction, new_transaction_id)) {
        std::cerr << "Failed to create transaction." << std::endl;
        return false;
    }

    // 获取新创建的交易记录信息
    if (!transaction_dao_.getTransactionById(new_transaction_id, new_transaction)) {
        std::cerr << "Failed to get new transaction." << std::endl;
        return false;
    }

    return true;
}

bool TransactionService::getTransactionById(int id, Transaction& transaction) {
    if (id <= 0) {
        std::cerr << "Invalid transaction ID." << std::endl;
        return false;
    }

    return transaction_dao_.getTransactionById(id, transaction);
}

TransactionPage TransactionService::getTransactionsByPage(const TransactionFilter& filter,
                                                              int page, int page_size) {
    // 验证分页参数
    if (page <= 0) {
        page = 1;
    }
    if (page_size <= 0 || page_size > 100) {
        page_size = 10;
    }

    // 验证时间格式
    if (!filter.from_time.empty() && !validateISO8601Time(filter.from_time)) {
        std::cerr << "Invalid from_time format." << std::endl;
        return TransactionPage();
    }
    if (!filter.to_time.empty() && !validateISO8601Time(filter.to_time)) {
        std::cerr << "Invalid to_time format." << std::endl;
        return TransactionPage();
    }

    // 创建一个可修改的副本
    TransactionFilter mutable_filter = filter;

    // 验证金额范围
    if (mutable_filter.amount_min < 0) {
        mutable_filter.amount_min = 0;
    }
    if (mutable_filter.amount_max < 0) {
        mutable_filter.amount_max = 0;
    }
    if (mutable_filter.amount_max > 0 && mutable_filter.amount_min > mutable_filter.amount_max) {
        std::cerr << "amount_min cannot be greater than amount_max." << std::endl;
        return TransactionPage();
    }

    return transaction_dao_.getTransactionsByPage(mutable_filter, page, page_size);
}

bool TransactionService::updateTransaction(int id, int account_id, int category_id, const std::string& type,
                                             double amount, const std::string& time, const std::string& note) {
    if (id <= 0) {
        std::cerr << "Invalid transaction ID." << std::endl;
        return false;
    }

    // 验证交易数据
    if (!validateTransactionData(account_id, category_id, type, amount, time)) {
        std::cerr << "Invalid transaction data." << std::endl;
        return false;
    }

    // 检查交易记录是否存在
    Transaction existing_transaction;
    if (!transaction_dao_.getTransactionById(id, existing_transaction)) {
        std::cerr << "Transaction not found." << std::endl;
        return false;
    }

    // 更新交易记录信息
    existing_transaction.setAccountId(account_id);
    existing_transaction.setCategoryId(category_id);
    existing_transaction.setType(type);
    existing_transaction.setAmount(amount);
    existing_transaction.setTime(time);
    existing_transaction.setNote(note);

    if (!transaction_dao_.updateTransaction(existing_transaction)) {
        std::cerr << "Failed to update transaction." << std::endl;
        return false;
    }

    return true;
}

bool TransactionService::deleteTransaction(int id) {
    if (id <= 0) {
        std::cerr << "Invalid transaction ID." << std::endl;
        return false;
    }

    // 检查交易记录是否存在
    Transaction existing_transaction;
    if (!transaction_dao_.getTransactionById(id, existing_transaction)) {
        std::cerr << "Transaction not found." << std::endl;
        return false;
    }

    // 调用 DAO 删除交易记录
    if (!transaction_dao_.deleteTransaction(id)) {
        std::cerr << "Failed to delete transaction." << std::endl;
        return false;
    }

    return true;
}

bool TransactionService::getMonthlySummary(const std::string& month, double& total_income, double& total_expense) {
    // 验证月份格式（YYYY-MM）
    std::regex month_regex(R"(\d{4}-\d{2})" ); 
    if (!std::regex_match(month, month_regex)) {
        std::cerr << "Invalid month format. Should be YYYY-MM." << std::endl;
        return false;
    }

    return transaction_dao_.getMonthlySummary(month, total_income, total_expense);
}

std::vector<std::pair<int, double>> TransactionService::getMonthlyExpenseByCategory(const std::string& month) {
    // 验证月份格式（YYYY-MM）
    std::regex month_regex(R"(\d{4}-\d{2})" ); 
    if (!std::regex_match(month, month_regex)) {
        std::cerr << "Invalid month format. Should be YYYY-MM." << std::endl;
        return std::vector<std::pair<int, double>>();
    }

    return transaction_dao_.getMonthlyExpenseByCategory(month);
}

bool TransactionService::validateTransactionData(int account_id, int category_id, const std::string& type,
                                                   double amount, const std::string& time) {
    // 验证账户 ID
    if (account_id <= 0) {
        std::cerr << "Invalid account ID." << std::endl;
        return false;
    }

    // 检查账户是否存在
    Account account;
    if (!account_dao_.getAccountById(account_id, account)) {
        std::cerr << "Account not found." << std::endl;
        return false;
    }

    // 验证分类 ID
    if (category_id <= 0) {
        std::cerr << "Invalid category ID." << std::endl;
        return false;
    }

    // 检查分类是否存在
    Category category;
    if (!category_dao_.getCategoryById(category_id, category)) {
        std::cerr << "Category not found." << std::endl;
        return false;
    }

    // 验证交易类型是否合法
    std::vector<std::string> valid_types = {"income", "expense"};
    if (std::find(valid_types.begin(), valid_types.end(), type) == valid_types.end()) {
        std::cerr << "Invalid transaction type." << std::endl;
        return false;
    }

    // 验证交易类型与分类类型是否匹配
    if (type != category.getType()) {
        std::cerr << "Transaction type does not match category type." << std::endl;
        return false;
    }

    // 验证金额是否合法
    if (amount <= 0) {
        std::cerr << "Transaction amount must be greater than 0." << std::endl;
        return false;
    }

    // 验证时间格式是否为 ISO8601
    if (!validateISO8601Time(time)) {
        std::cerr << "Invalid time format. Should be ISO8601." << std::endl;
        return false;
    }

    return true;
}

bool TransactionService::validateISO8601Time(const std::string& time) {
    // 验证 ISO8601 时间格式（YYYY-MM-DDTHH:MM:SS）
    std::regex iso8601_regex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2})" ); 
    return std::regex_match(time, iso8601_regex);
}

} // namespace accounting
