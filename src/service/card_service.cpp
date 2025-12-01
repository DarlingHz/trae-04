#include "card_service.h"
#include "util/time.h"

CardService::CardService(const std::string& db_path) : card_dao_(db_path), tag_dao_(db_path) {
}

Result<int> CardService::CreateCard(int user_id, const std::string& title, const std::string& content,
                                        const std::vector<std::string>& tag_names) {
  // 验证标题是否为空
  if (title.empty()) {
    return Result<int>::Error("Title cannot be empty");
  }
  
  // 处理标签
  std::vector<int> tag_ids = ProcessTags(user_id, tag_names);
  
  // 创建卡片
  Card card;
  card.user_id = user_id;
  card.title = title;
  card.content = content;
  card.tags = TagsToString(tag_ids);
  card.is_pinned = 0;
  card.is_deleted = 0;
  card.created_at = TimeUtil::GetCurrentTimestamp();
  card.updated_at = card.created_at;
  
  int card_id = card_dao_.CreateCard(card);
  if (card_id == -1) {
    return Result<int>::Error("Failed to create card");
  }
  
  return Result<int>::Success(card_id);
}

Result<bool> CardService::UpdateCard(int user_id, int card_id, const std::string& title, const std::string& content,
                                         const std::vector<std::string>& tag_names) {
  // 验证标题是否为空
  if (title.empty()) {
    return Result<bool>::Error("Title cannot be empty");
  }
  
  // 检查卡片是否存在且属于该用户
  auto card_result = GetCardById(user_id, card_id);
  if (!card_result.IsSuccess()) {
    return Result<bool>::Error(card_result.GetError());
  }
  
  // 处理标签
  std::vector<int> tag_ids = ProcessTags(user_id, tag_names);
  
  // 更新卡片
  Card card = card_result.GetData();
  card.title = title;
  card.content = content;
  card.tags = TagsToString(tag_ids);
  card.updated_at = TimeUtil::GetCurrentTimestamp();
  
  if (!card_dao_.UpdateCard(card)) {
    return Result<bool>::Error("Failed to update card");
  }
  
  return Result<bool>::Success(true);
}

Result<bool> CardService::DeleteCard(int user_id, int card_id) {
  // 检查卡片是否存在且属于该用户
  auto card_result = GetCardById(user_id, card_id);
  if (!card_result.IsSuccess()) {
    return Result<bool>::Error(card_result.GetError());
  }
  
  // 删除卡片
  if (!card_dao_.DeleteCard(card_id)) {
    return Result<bool>::Error("Failed to delete card");
  }
  
  return Result<bool>::Success(true);
}

Result<Card> CardService::GetCardById(int user_id, int card_id) {
  // 获取卡片
  auto card = card_dao_.GetCardById(card_id);
  if (!card) {
    return Result<Card>::Error("Card not found");
  }
  
  // 检查卡片是否属于该用户
  if (card->user_id != user_id) {
    return Result<Card>::Error("Permission denied");
  }
  
  return Result<Card>::Success(*card);
}

Result<CardList> CardService::GetCardList(int user_id, int page, int page_size, const std::string& sort,
                                               const std::vector<std::string>& tag_names, const std::string& search) {
  // 验证分页参数
  if (page < 1) page = 1;
  if (page_size < 1 || page_size > 100) page_size = 10;
  
  // 处理标签名称为标签ID
  std::vector<int> tag_ids;
  for (const auto& tag_name : tag_names) {
    auto tag = tag_dao_.GetTagByName(user_id, tag_name);
    if (tag) {
      tag_ids.push_back(tag->tag_id);
    }
  }
  
  // 计算偏移量
  int offset = (page - 1) * page_size;
  
  // 获取卡片列表和总数
  std::vector<Card> cards = card_dao_.GetCardList(user_id, offset, page_size, sort, tag_ids, search);
  int total = card_dao_.GetCardCount(user_id, tag_ids, search);
  
  // 构造返回结果
  CardList card_list;
  card_list.cards = cards;
  card_list.total = total;
  card_list.page = page;
  card_list.size = page_size;
  
  return Result<CardList>::Success(card_list);
}

std::vector<int> CardService::ProcessTags(int user_id, const std::vector<std::string>& tag_names) {
  std::vector<int> tag_ids;
  
  for (const auto& tag_name : tag_names) {
    // 跳过空标签名称
    if (tag_name.empty()) continue;
    
    // 检查标签是否已存在
    auto existing_tag = tag_dao_.GetTagByName(user_id, tag_name);
    if (existing_tag) {
      tag_ids.push_back(existing_tag->tag_id);
    } else {
      // 创建新标签
      Tag new_tag;
      new_tag.user_id = user_id;
      new_tag.name = tag_name;
      new_tag.created_at = TimeUtil::GetCurrentTimestamp();
      new_tag.updated_at = new_tag.created_at;
      
      int tag_id = tag_dao_.CreateTag(new_tag);
      if (tag_id != -1) {
        tag_ids.push_back(tag_id);
      }
    }
  }
  
  return tag_ids;
}

std::vector<std::string> CardService::TagsToString(const std::vector<int>& tag_ids) {
  std::vector<std::string> tags;
  
  for (int tag_id : tag_ids) {
    auto tag_result = tag_dao_.GetTagById(tag_id);
    if (tag_result.has_value()) {
      tags.push_back(tag_result.value().name);
    }
  }
  
  return tags;
}
