#include "TransactionDAO.h"
#include "Database.h"
#include <sqlite3.h>
#include <sstream>
#include <iostream>

namespace accounting {

bool TransactionDAO::createTransaction(const Transaction& transaction, int& new_transaction_id) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "INSERT INTO transactions (account_id, category_id, type, amount, time, note) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, transaction.getAccountId());
    sqlite3_bind_int(stmt, 2, transaction.getCategoryId());
    sqlite3_bind_text(stmt, 3, transaction.getType().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction.getAmount());
    sqlite3_bind_text(stmt, 5, transaction.getTime().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, transaction.getNote().c_str(), -1, SQLITE_TRANSIENT);

    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 获取新插入的 ID
    new_transaction_id = sqlite3_last_insert_rowid(conn);

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

bool TransactionDAO::getTransactionById(int id, Transaction& transaction) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "SELECT id, account_id, category_id, type, amount, time, note FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, id);

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    // 读取结果
    int transaction_id = sqlite3_column_int(stmt, 0);
    int account_id = sqlite3_column_int(stmt, 1);
    int category_id = sqlite3_column_int(stmt, 2);
    const unsigned char* type = sqlite3_column_text(stmt, 3);
    double amount = sqlite3_column_double(stmt, 4);
    const unsigned char* time = sqlite3_column_text(stmt, 5);
    const unsigned char* note = sqlite3_column_text(stmt, 6);

    transaction = Transaction(transaction_id, account_id, category_id,
                               reinterpret_cast<const char*>(type), amount,
                               reinterpret_cast<const char*>(time), reinterpret_cast<const char*>(note));

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

TransactionPage TransactionDAO::getTransactionsByPage(const TransactionFilter& filter,
                                                          int page, int page_size) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    TransactionPage transaction_page;
    transaction_page.page = page;
    transaction_page.page_size = page_size;

    if (conn == nullptr) {
        return transaction_page;
    }

    // 构建查询语句
    std::stringstream sql_ss;
    sql_ss << "SELECT id, account_id, category_id, type, amount, time, note FROM transactions WHERE 1=1";

    // 添加过滤条件
    if (!filter.from_time.empty()) {
        sql_ss << " AND time >= ?";
    }
    if (!filter.to_time.empty()) {
        sql_ss << " AND time <= ?";
    }
    if (filter.account_id > 0) {
        sql_ss << " AND account_id = ?";
    }
    if (filter.category_id > 0) {
        sql_ss << " AND category_id = ?";
    }
    if (!filter.type.empty()) {
        sql_ss << " AND type = ?";
    }
    if (filter.amount_min > 0) {
        sql_ss << " AND amount >= ?";
    }
    if (filter.amount_max > 0) {
        sql_ss << " AND amount <= ?";
    }

    // 添加排序
    sql_ss << " ORDER BY time DESC";

    // 添加分页
    sql_ss << " LIMIT ? OFFSET ?;";

    std::string sql = sql_ss.str();
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return transaction_page;
    }

    // 绑定参数
    int param_index = 1;
    if (!filter.from_time.empty()) {
        sqlite3_bind_text(stmt, param_index++, filter.from_time.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!filter.to_time.empty()) {
        sqlite3_bind_text(stmt, param_index++, filter.to_time.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (filter.account_id > 0) {
        sqlite3_bind_int(stmt, param_index++, filter.account_id);
    }
    if (filter.category_id > 0) {
        sqlite3_bind_int(stmt, param_index++, filter.category_id);
    }
    if (!filter.type.empty()) {
        sqlite3_bind_text(stmt, param_index++, filter.type.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (filter.amount_min > 0) {
        sqlite3_bind_double(stmt, param_index++, filter.amount_min);
    }
    if (filter.amount_max > 0) {
        sqlite3_bind_double(stmt, param_index++, filter.amount_max);
    }

    // 绑定分页参数
    sqlite3_bind_int(stmt, param_index++, page_size);
    sqlite3_bind_int(stmt, param_index++, (page - 1) * page_size);

    // 遍历结果集
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int account_id = sqlite3_column_int(stmt, 1);
        int category_id = sqlite3_column_int(stmt, 2);
        const unsigned char* type = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        const unsigned char* time = sqlite3_column_text(stmt, 5);
        const unsigned char* note = sqlite3_column_text(stmt, 6);

        transaction_page.transactions.emplace_back(id, account_id, category_id,
                                                      reinterpret_cast<const char*>(type), amount,
                                                      reinterpret_cast<const char*>(time), reinterpret_cast<const char*>(note));
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to step through results: " << sqlite3_errmsg(conn) << std::endl;
    }

    // 清理
    sqlite3_finalize(stmt);

    // 获取总记录数
    std::stringstream count_sql_ss;
    count_sql_ss << "SELECT COUNT(*) FROM transactions WHERE 1=1";

    // 添加相同的过滤条件
    if (!filter.from_time.empty()) {
        count_sql_ss << " AND time >= ?";
    }
    if (!filter.to_time.empty()) {
        count_sql_ss << " AND time <= ?";
    }
    if (filter.account_id > 0) {
        count_sql_ss << " AND account_id = ?";
    }
    if (filter.category_id > 0) {
        count_sql_ss << " AND category_id = ?";
    }
    if (!filter.type.empty()) {
        count_sql_ss << " AND type = ?";
    }
    if (filter.amount_min > 0) {
        count_sql_ss << " AND amount >= ?";
    }
    if (filter.amount_max > 0) {
        count_sql_ss << " AND amount <= ?";
    }

    count_sql_ss << ";";

    std::string count_sql = count_sql_ss.str();
    sqlite3_stmt* count_stmt = nullptr;

    rc = sqlite3_prepare_v2(conn, count_sql.c_str(), -1, &count_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare count statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(count_stmt);
        return transaction_page;
    }

    // 绑定相同的参数
    param_index = 1;
    if (!filter.from_time.empty()) {
        sqlite3_bind_text(count_stmt, param_index++, filter.from_time.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!filter.to_time.empty()) {
        sqlite3_bind_text(count_stmt, param_index++, filter.to_time.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (filter.account_id > 0) {
        sqlite3_bind_int(count_stmt, param_index++, filter.account_id);
    }
    if (filter.category_id > 0) {
        sqlite3_bind_int(count_stmt, param_index++, filter.category_id);
    }
    if (!filter.type.empty()) {
        sqlite3_bind_text(count_stmt, param_index++, filter.type.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (filter.amount_min > 0) {
        sqlite3_bind_double(count_stmt, param_index++, filter.amount_min);
    }
    if (filter.amount_max > 0) {
        sqlite3_bind_double(count_stmt, param_index++, filter.amount_max);
    }

    // 执行查询
    rc = sqlite3_step(count_stmt);
    if (rc == SQLITE_ROW) {
        transaction_page.total_count = sqlite3_column_int(count_stmt, 0);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to step through count results: " << sqlite3_errmsg(conn) << std::endl;
    }

    // 清理
    sqlite3_finalize(count_stmt);

    return transaction_page;
}

bool TransactionDAO::updateTransaction(const Transaction& transaction) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "UPDATE transactions SET account_id = ?, category_id = ?, type = ?, amount = ?, time = ?, note = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, transaction.getAccountId());
    sqlite3_bind_int(stmt, 2, transaction.getCategoryId());
    sqlite3_bind_text(stmt, 3, transaction.getType().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction.getAmount());
    sqlite3_bind_text(stmt, 5, transaction.getTime().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, transaction.getNote().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, transaction.getId());

    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 检查是否有行被更新
    int changes = sqlite3_changes(conn);
    if (changes == 0) {
        sqlite3_finalize(stmt);
        return false;
    }

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

bool TransactionDAO::deleteTransaction(int id) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "DELETE FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, id);

    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 检查是否有行被删除
    int changes = sqlite3_changes(conn);
    if (changes == 0) {
        sqlite3_finalize(stmt);
        return false;
    }

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

bool TransactionDAO::getMonthlySummary(const std::string& month, double& total_income, double& total_expense) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    // 构建月份的时间范围
    std::string from_time = month + "-01T00:00:00";
    std::string to_time = month + "-31T23:59:59";

    // 查询总收入
    const char* income_sql = "SELECT SUM(amount) FROM transactions WHERE type = 'income' AND time >= ? AND time <= ?;";
    sqlite3_stmt* income_stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, income_sql, -1, &income_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare income statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(income_stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(income_stmt, 1, from_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(income_stmt, 2, to_time.c_str(), -1, SQLITE_TRANSIENT);

    // 执行查询
    rc = sqlite3_step(income_stmt);
    if (rc == SQLITE_ROW) {
        if (sqlite3_column_type(income_stmt, 0) == SQLITE_NULL) {
            total_income = 0.0;
        } else {
            total_income = sqlite3_column_double(income_stmt, 0);
        }
    } else {
        std::cerr << "Failed to step through income results: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(income_stmt);
        return false;
    }

    // 清理
    sqlite3_finalize(income_stmt);

    // 查询总支出
    const char* expense_sql = "SELECT SUM(amount) FROM transactions WHERE type = 'expense' AND time >= ? AND time <= ?;";
    sqlite3_stmt* expense_stmt = nullptr;

    rc = sqlite3_prepare_v2(conn, expense_sql, -1, &expense_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare expense statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(expense_stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(expense_stmt, 1, from_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(expense_stmt, 2, to_time.c_str(), -1, SQLITE_TRANSIENT);

    // 执行查询
    rc = sqlite3_step(expense_stmt);
    if (rc == SQLITE_ROW) {
        if (sqlite3_column_type(expense_stmt, 0) == SQLITE_NULL) {
            total_expense = 0.0;
        } else {
            total_expense = sqlite3_column_double(expense_stmt, 0);
        }
    } else {
        std::cerr << "Failed to step through expense results: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(expense_stmt);
        return false;
    }

    // 清理
    sqlite3_finalize(expense_stmt);

    return true;
}

std::vector<std::pair<int, double>> TransactionDAO::getMonthlyExpenseByCategory(const std::string& month) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    std::vector<std::pair<int, double>> expense_by_category;

    if (conn == nullptr) {
        return expense_by_category;
    }

    // 构建月份的时间范围
    std::string from_time = month + "-01T00:00:00";
    std::string to_time = month + "-31T23:59:59";

    // 查询每个分类的支出总额
    const char* sql = "SELECT category_id, SUM(amount) FROM transactions WHERE type = 'expense' AND time >= ? AND time <= ? GROUP BY category_id;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return expense_by_category;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, from_time.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, to_time.c_str(), -1, SQLITE_TRANSIENT);

    // 遍历结果集
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int category_id = sqlite3_column_int(stmt, 0);
        double amount = 0.0;
        if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
            amount = sqlite3_column_double(stmt, 1);
        }

        expense_by_category.emplace_back(category_id, amount);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to step through results: " << sqlite3_errmsg(conn) << std::endl;
    }

    // 清理
    sqlite3_finalize(stmt);

    return expense_by_category;
}

} // namespace accounting
