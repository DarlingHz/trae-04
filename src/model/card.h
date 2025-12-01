#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Card {
  int card_id;
  int user_id;
  std::string title;
  std::string content;
  std::vector<std::string> tags;
  bool is_pinned;
  bool is_deleted;
  int64_t created_at;
  int64_t updated_at;
};

struct CardList {
  int total;
  int page;
  int size;
  std::vector<Card> cards;
};

// JSON序列化函数
void to_json(json& j, const Card& card);
void to_json(json& j, const CardList& card_list);
