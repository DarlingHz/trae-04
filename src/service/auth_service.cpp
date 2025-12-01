#include "auth_service.h"
#include "util/time.h"

AuthService::AuthService(const std::string& db_path, const std::string& jwt_secret)
  : user_dao_(db_path), jwt_util_(jwt_secret) {
}

Result<UserWithToken> AuthService::Register(const std::string& email, const std::string& password) {
  // 验证邮箱格式（简单验证）
  if (email.find('@') == std::string::npos) {
    return Result<UserWithToken>::Error("Invalid email format");
  }
  
  // 验证密码长度
  if (password.length() < 6) {
    return Result<UserWithToken>::Error("Password must be at least 6 characters");
  }
  
  // 检查邮箱是否已存在
  auto existing_user = user_dao_.GetUserByEmail(email);
  if (existing_user) {
    return Result<UserWithToken>::Error("Email already registered");
  }
  
  // 生成密码哈希和盐
  auto password_data = password_util_.GeneratePasswordHash(password);
  
  // 创建用户
  User user;
  user.email = email;
  user.password_hash = password_data.first;
  user.password_salt = password_data.second;
  user.created_at = TimeUtil::GetCurrentTimestamp();
  user.updated_at = user.created_at;
  
  if (!user_dao_.CreateUser(user)) {
    return Result<UserWithToken>::Error("Failed to create user");
  }
  
  // 获取刚创建的用户
  auto new_user = user_dao_.GetUserByEmail(email);
  if (!new_user) {
    return Result<UserWithToken>::Error("Failed to retrieve new user");
  }
  
  // 生成JWT令牌
  std::map<std::string, std::string> claims;
  claims["user_id"] = std::to_string(new_user->user_id);
  claims["email"] = new_user->email;
  std::string token = jwt_util_.GenerateToken(claims);
  
  // 构造返回结果
  UserWithToken user_with_token;
  user_with_token.user_id = new_user->user_id;
  user_with_token.email = new_user->email;
  user_with_token.token = token;
  
  return Result<UserWithToken>::Success(user_with_token);
}

Result<UserWithToken> AuthService::Login(const std::string& email, const std::string& password) {
  // 检查用户是否存在
  auto user = user_dao_.GetUserByEmail(email);
  if (!user) {
    return Result<UserWithToken>::Error("Invalid email or password");
  }
  
  // 验证密码
  bool is_password_valid = password_util_.VerifyPassword(password, user->password_hash, user->password_salt);
  if (!is_password_valid) {
    return Result<UserWithToken>::Error("Invalid email or password");
  }
  
  // 生成JWT令牌
  std::map<std::string, std::string> claims;
  claims["user_id"] = std::to_string(user->user_id);
  claims["email"] = user->email;
  std::string token = jwt_util_.GenerateToken(claims);
  
  // 构造返回结果
  UserWithToken user_with_token;
  user_with_token.user_id = user->user_id;
  user_with_token.email = user->email;
  user_with_token.token = token;
  
  return Result<UserWithToken>::Success(user_with_token);
}
