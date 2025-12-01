#pragma once

#include "base_dao.h"
#include "model/user.h"
#include <optional>

class UserDao : public BaseDao {
public:
  // 构造函数
  UserDao(const std::string& db_path);
  
  // 创建用户表
  void CreateUserTable();
  
  // 创建用户
  bool CreateUser(const User& user);
  
  // 根据邮箱查询用户
  std::optional<User> GetUserByEmail(const std::string& email);
  
  // 根据用户ID查询用户
  std::optional<User> GetUserById(int user_id);
};
