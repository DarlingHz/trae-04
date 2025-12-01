#pragma once

#include "dao/user_dao.h"
#include "auth/simple_password.h"
#include "auth/simple_jwt.h"
#include "result.h"
#include "model/user.h"

class AuthService {
public:
  // 构造函数
  AuthService(const std::string& db_path, const std::string& jwt_secret);
  
  // 用户注册
  Result<UserWithToken> Register(const std::string& email, const std::string& password);
  
  // 用户登录
  Result<UserWithToken> Login(const std::string& email, const std::string& password);
  
private:
  UserDao user_dao_;
  SimplePassword password_util_;
  SimpleJWT jwt_util_;
};
