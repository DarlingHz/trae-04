#include "BudgetDAO.h"
#include "Database.h"
#include <sqlite3.h>
#include <iostream>

namespace accounting {

bool BudgetDAO::setBudget(const Budget& budget) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    // 使用 INSERT OR REPLACE 来实现设置或更新预算
    const char* sql = "INSERT OR REPLACE INTO budgets (id, month, category_id, limit) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    if (budget.getId() > 0) {
        sqlite3_bind_int(stmt, 1, budget.getId());
    } else {
        sqlite3_bind_null(stmt, 1);
    }
    sqlite3_bind_text(stmt, 2, budget.getMonth().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, budget.getCategoryId());
    sqlite3_bind_double(stmt, 4, budget.getLimit());

    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

bool BudgetDAO::getBudgetById(int id, Budget& budget) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "SELECT id, month, category_id, limit FROM budgets WHERE id = ?;";
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
    int budget_id = sqlite3_column_int(stmt, 0);
    const unsigned char* month = sqlite3_column_text(stmt, 1);
    int category_id = sqlite3_column_int(stmt, 2);
    double limit = sqlite3_column_double(stmt, 3);

    budget = Budget(budget_id, reinterpret_cast<const char*>(month), category_id, limit);

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

std::vector<Budget> BudgetDAO::getBudgetsByMonth(const std::string& month) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    std::vector<Budget> budgets;

    if (conn == nullptr) {
        return budgets;
    }

    const char* sql = "SELECT id, month, category_id, limit FROM budgets WHERE month = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return budgets;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, month.c_str(), -1, SQLITE_TRANSIENT);

    // 遍历结果集
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* month_str = sqlite3_column_text(stmt, 1);
        int category_id = sqlite3_column_int(stmt, 2);
        double limit = sqlite3_column_double(stmt, 3);

        budgets.emplace_back(id, reinterpret_cast<const char*>(month_str), category_id, limit);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to step through results: " << sqlite3_errmsg(conn) << std::endl;
    }

    // 清理
    sqlite3_finalize(stmt);

    return budgets;
}

bool BudgetDAO::getBudgetByMonthAndCategory(const std::string& month, int category_id, Budget& budget) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "SELECT id, month, category_id, limit FROM budgets WHERE month = ? AND category_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, month.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, category_id);

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    // 读取结果
    int budget_id = sqlite3_column_int(stmt, 0);
    const unsigned char* month_str = sqlite3_column_text(stmt, 1);
    int cat_id = sqlite3_column_int(stmt, 2);
    double limit = sqlite3_column_double(stmt, 3);

    budget = Budget(budget_id, reinterpret_cast<const char*>(month_str), cat_id, limit);

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

bool BudgetDAO::deleteBudget(int id) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "DELETE FROM budgets WHERE id = ?;";
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

bool BudgetDAO::deleteBudgetsByMonth(const std::string& month) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "DELETE FROM budgets WHERE month = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, month.c_str(), -1, SQLITE_TRANSIENT);

    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

} // namespace accounting
