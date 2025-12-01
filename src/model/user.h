#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct User {
  int user_id;
  std::string email;
  std::string password_hash;
  std::string password_salt;
  int64_t created_at;
  int64_t updated_at;
};

struct UserWithToken {
  int user_id;
  std::string email;
  std::string token;
};

// JSON序列化函数
void to_json(json& j, const UserWithToken& user);
