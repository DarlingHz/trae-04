#include "base_dao.h"
#include <iostream>

BaseDao::BaseDao(const std::string& db_path) {
  // 打开数据库
  int rc = sqlite3_open(db_path.c_str(), &db_);
  if (rc != SQLITE_OK) {
    std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << std::endl;
    sqlite3_close(db_);
    db_ = nullptr;
  }

  // 启用外键约束
  char* errmsg = nullptr;
  rc = sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "Failed to enable foreign keys: " << errmsg << std::endl;
    sqlite3_free(errmsg);
  }
}

BaseDao::~BaseDao() {
  // 关闭数据库
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

bool BaseDao::BeginTransaction() {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "Failed to begin transaction: " << errmsg << std::endl;
    sqlite3_free(errmsg);
    return false;
  }

  return true;
}

bool BaseDao::CommitTransaction() {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_, "COMMIT TRANSACTION;", nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "Failed to commit transaction: " << errmsg << std::endl;
    sqlite3_free(errmsg);
    return false;
  }

  return true;
}

bool BaseDao::RollbackTransaction() {
  if (db_ == nullptr) {
    std::cerr << "Database not open" << std::endl;
    return false;
  }

  char* errmsg = nullptr;
  int rc = sqlite3_exec(db_, "ROLLBACK TRANSACTION;", nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    std::cerr << "Failed to rollback transaction: " << errmsg << std::endl;
    sqlite3_free(errmsg);
    return false;
  }

  return true;
}
