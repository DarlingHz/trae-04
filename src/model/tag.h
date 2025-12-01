#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Tag {
  int tag_id;
  int user_id;
  std::string name;
  int64_t created_at;
  int64_t updated_at;
};

struct TagWithCount {
  int tag_id;
  std::string name;
  int card_count;
};

// JSON序列化函数
void to_json(json& j, const TagWithCount& tag);
