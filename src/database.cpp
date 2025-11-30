#include "database.h"
#include <iostream>
#include <sstream>

Database::Database() : db_(nullptr), is_open_(false) {
}

Database::~Database() {
    close();
}

bool Database::open(const std::string& db_path) {
    if (is_open_) {
        close();
    }
    
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    
    is_open_ = true;
    std::cout << "数据库连接成功" << std::endl;
    return true;
}

void Database::close() {
    if (is_open_ && db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
        is_open_ = false;
        std::cout << "数据库连接已关闭" << std::endl;
    }
}

std::vector<std::map<std::string, std::string>> Database::execute_query(const std::string& sql) {
    std::vector<std::map<std::string, std::string>> result;
    
    if (!is_open_) {
        std::cerr << "数据库未连接" << std::endl;
        return result;
    }
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL 查询准备失败: " << sqlite3_errmsg(db_) << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        return result;
    }
    
    // 获取列数
    int column_count = sqlite3_column_count(stmt);
    std::vector<std::string> column_names;
    
    for (int i = 0; i < column_count; ++i) {
        column_names.push_back(sqlite3_column_name(stmt, i));
    }
    
    // 执行查询并获取结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::map<std::string, std::string> row;
        
        for (int i = 0; i < column_count; ++i) {
            const char* value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[column_names[i]] = (value != nullptr) ? value : "";
        }
        
        result.push_back(row);
    }
    
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL 查询执行失败: " << sqlite3_errmsg(db_) << std::endl;
    }
    
    // 清理语句
    sqlite3_finalize(stmt);
    
    return result;
}

bool Database::execute_non_query(const std::string& sql) {
    if (!is_open_) {
        std::cerr << "数据库未连接" << std::endl;
        return false;
    }
    
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL 执行失败: " << err_msg << std::endl;
        std::cerr << "SQL: " << sql << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

bool Database::begin_transaction() {
    return execute_non_query("BEGIN TRANSACTION;");
}

bool Database::commit_transaction() {
    return execute_non_query("COMMIT;");
}

bool Database::rollback_transaction() {
    return execute_non_query("ROLLBACK;");
}

int64_t Database::get_last_insert_rowid() {
    if (!is_open_) {
        return -1;
    }
    
    return sqlite3_last_insert_rowid(db_);
}

int Database::get_affected_rows() {
    if (!is_open_) {
        return -1;
    }
    
    return sqlite3_changes(db_);
}