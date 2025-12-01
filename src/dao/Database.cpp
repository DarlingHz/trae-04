#include "Database.h"
#include <iostream>

namespace accounting {

Database::Database() {
}

Database::~Database() {
    close();
}

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

bool Database::open(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (db_ != nullptr) {
        std::cout << "Database is already open." << std::endl;
        return true;
    }

    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    // 创建表
    createTables();

    std::cout << "Database opened successfully." << std::endl;
    return true;
}

void Database::close() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
        std::cout << "Database closed." << std::endl;
    }
}

sqlite3* Database::getConnection() {
    return db_;
}

sqlite3_stmt* Database::executeQuery(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (db_ == nullptr) {
        std::cerr << "Database is not open." << std::endl;
        return nullptr;
    }

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return nullptr;
    }

    return stmt;
}

bool Database::executeUpdate(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (db_ == nullptr) {
        std::cerr << "Database is not open." << std::endl;
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

void Database::createTables() {
    // 创建 accounts 表
    std::string create_accounts = R"(
        CREATE TABLE IF NOT EXISTS accounts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            initial_balance REAL DEFAULT 0.0
        );
    )";
    executeUpdate(create_accounts);

    // 创建 categories 表
    std::string create_categories = R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            type TEXT NOT NULL CHECK(type IN ('income', 'expense'))
        );
    )";
    executeUpdate(create_categories);

    // 创建 transactions 表
    std::string create_transactions = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            account_id INTEGER NOT NULL,
            category_id INTEGER NOT NULL,
            type TEXT NOT NULL CHECK(type IN ('income', 'expense')),
            amount REAL NOT NULL CHECK(amount > 0),
            time TEXT NOT NULL,
            note TEXT,
            FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE,
            FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE CASCADE
        );
    )";
    executeUpdate(create_transactions);

    // 创建 budgets 表
    std::string create_budgets = R"(
        CREATE TABLE IF NOT EXISTS budgets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            month TEXT NOT NULL,
            category_id INTEGER NOT NULL,
            limit REAL NOT NULL CHECK(limit >= 0),
            FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE CASCADE,
            UNIQUE(month, category_id)
        );
    )";
    executeUpdate(create_budgets);

    // 创建索引
    std::string create_transactions_time_index = "CREATE INDEX IF NOT EXISTS idx_transactions_time ON transactions(time);";
    executeUpdate(create_transactions_time_index);

    std::string create_transactions_account_index = "CREATE INDEX IF NOT EXISTS idx_transactions_account ON transactions(account_id);";
    executeUpdate(create_transactions_account_index);

    std::string create_transactions_category_index = "CREATE INDEX IF NOT EXISTS idx_transactions_category ON transactions(category_id);";
    executeUpdate(create_transactions_category_index);
}

} // namespace accounting
