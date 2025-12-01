#include "user_dao.h"
#include <iostream>

UserDao::UserDao(const std::string& db_path) : BaseDao(db_path) {
  CreateUserTable();
}

void UserDao::CreateUserTable() {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return;
  }

  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_, 
    "CREATE TABLE IF NOT EXISTS users ("
    "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "email TEXT UNIQUE NOT NULL,"
    "password_hash TEXT NOT NULL,"
    "password_salt TEXT NOT NULL,"
    "created_at INTEGER NOT NULL,"
    "updated_at INTEGER NOT NULL"
    ");", 
    nullptr, nullptr, &errmsg);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to create users table: " << errmsg << std::endl;
    sqlite3_free(errmsg);
  }
}

bool UserDao::CreateUser(const User& user) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "INSERT INTO users (email, password_hash, password_salt, created_at, updated_at) VALUES (?, ?, ?, ?, ?)", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return false;
  }

  // 绑定参数
  sqlite3_bind_text(stmt, 1, user.email.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, user.password_hash.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, user.password_salt.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 4, user.created_at);
  sqlite3_bind_int64(stmt, 5, user.updated_at);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
    sqlite3_finalize(stmt);
    return false;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return true;
}

std::optional<User> UserDao::GetUserByEmail(const std::string& email) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return std::nullopt;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "SELECT * FROM users WHERE email = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return std::nullopt;
  }

  // 绑定参数
  sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    User user;
    user.user_id = sqlite3_column_int(stmt, 0);
    user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    user.password_salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    user.created_at = sqlite3_column_int64(stmt, 4);
    user.updated_at = sqlite3_column_int64(stmt, 5);

    // 清理语句
    sqlite3_finalize(stmt);
    return user;
  } else if (rc == SQLITE_DONE) {
    // 没有找到用户
  } else {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return std::nullopt;
}

std::optional<User> UserDao::GetUserById(int user_id) {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return std::nullopt;
  }

  sqlite3_stmt* stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, "SELECT * FROM users WHERE user_id = ?", -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
    return std::nullopt;
  }

  // 绑定参数
  sqlite3_bind_int(stmt, 1, user_id);

  // 执行语句
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    User user;
    user.user_id = sqlite3_column_int(stmt, 0);
    user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    user.password_salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    user.created_at = sqlite3_column_int64(stmt, 4);
    user.updated_at = sqlite3_column_int64(stmt, 5);

    // 清理语句
    sqlite3_finalize(stmt);
    return user;
  } else if (rc == SQLITE_DONE) {
    // 没有找到用户
  } else {
    std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
  }

  // 清理语句
  sqlite3_finalize(stmt);
  return std::nullopt;
}
