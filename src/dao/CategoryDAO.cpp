#include "CategoryDAO.h"
#include "Database.h"
#include <sqlite3.h>
#include <iostream>

namespace accounting {

bool CategoryDAO::createCategory(const Category& category, int& new_category_id) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "INSERT INTO categories (name, type) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, category.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, category.getType().c_str(), -1, SQLITE_TRANSIENT);

    // 执行语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 获取新插入的 ID
    new_category_id = sqlite3_last_insert_rowid(conn);

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

bool CategoryDAO::getCategoryById(int id, Category& category) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "SELECT id, name, type FROM categories WHERE id = ?;";
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
    int category_id = sqlite3_column_int(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* type = sqlite3_column_text(stmt, 2);

    category = Category(category_id, reinterpret_cast<const char*>(name),
                         reinterpret_cast<const char*>(type));

    // 清理
    sqlite3_finalize(stmt);

    return true;
}

std::vector<Category> CategoryDAO::getAllCategories(const std::string& type_filter) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    std::vector<Category> categories;

    if (conn == nullptr) {
        return categories;
    }

    std::string sql = "SELECT id, name, type FROM categories";
    if (!type_filter.empty()) {
        sql += " WHERE type = ?";
    }
    sql += ";";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return categories;
    }

    // 绑定参数
    if (!type_filter.empty()) {
        sqlite3_bind_text(stmt, 1, type_filter.c_str(), -1, SQLITE_TRANSIENT);
    }

    // 遍历结果集
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* type = sqlite3_column_text(stmt, 2);

        categories.emplace_back(id, reinterpret_cast<const char*>(name),
                                 reinterpret_cast<const char*>(type));
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to step through results: " << sqlite3_errmsg(conn) << std::endl;
    }

    // 清理
    sqlite3_finalize(stmt);

    return categories;
}

bool CategoryDAO::updateCategory(const Category& category) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "UPDATE categories SET name = ?, type = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(conn) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, category.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, category.getType().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, category.getId());

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

bool CategoryDAO::deleteCategory(int id) {
    Database& db = Database::getInstance();
    sqlite3* conn = db.getConnection();

    if (conn == nullptr) {
        return false;
    }

    const char* sql = "DELETE FROM categories WHERE id = ?;";
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

} // namespace accounting
