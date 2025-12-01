#pragma once

#include "dao/card_dao.h"
#include "dao/tag_dao.h"
#include "result.h"
#include "model/card.h"
#include <vector>

class CardService {
public:
  // 构造函数
  CardService(const std::string& db_path);
  
  // 创建卡片
  Result<int> CreateCard(int user_id, const std::string& title, const std::string& content,
                          const std::vector<std::string>& tag_names);
  
  // 更新卡片
  Result<bool> UpdateCard(int user_id, int card_id, const std::string& title, const std::string& content,
                           const std::vector<std::string>& tag_names);
  
  // 删除卡片
  Result<bool> DeleteCard(int user_id, int card_id);
  
  // 获取卡片详情
  Result<Card> GetCardById(int user_id, int card_id);
  
  // 获取卡片列表
  Result<CardList> GetCardList(int user_id, int page, int page_size, const std::string& sort,
                                 const std::vector<std::string>& tag_names, const std::string& search);
  
private:
  // 处理标签：创建不存在的标签并返回标签ID列表
  std::vector<int> ProcessTags(int user_id, const std::vector<std::string>& tag_names);
  
  // 将标签ID列表转换为逗号分隔的字符串
  std::vector<std::string> TagsToString(const std::vector<int>& tag_ids);
  
  CardDao card_dao_;
  TagDao tag_dao_;
};
