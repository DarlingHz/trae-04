#pragma once

#include <sqlite3.h>
#include <string>

class BaseDao {
public:
  // 构造函数
  BaseDao(const std::string& db_path);
  
  // 析构函数
  virtual ~BaseDao();
  
  // 开始事务
  bool BeginTransaction();
  
  // 提交事务
  bool CommitTransaction();
  
  // 回滚事务
  bool RollbackTransaction();
  
protected:
  sqlite3* db_;
};
