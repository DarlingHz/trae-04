#pragma once

#include "base_dao.h"
#include "model/card.h"
#include <optional>
#include <vector>

class CardDao : public BaseDao {
public:
  // 构造函数
  CardDao(const std::string& db_path);
  
  // 创建卡片表
  void CreateCardTable();
  
  // 创建卡片
  int CreateCard(const Card& card);
  
  // 更新卡片
  bool UpdateCard(const Card& card);
  
  // 删除卡片（软删除）
  bool DeleteCard(int card_id);
  
  // 根据卡片ID查询卡片
  std::optional<Card> GetCardById(int card_id);
  
  // 查询用户卡片列表
  std::vector<Card> GetCardList(int user_id, int offset, int limit, const std::string& sort,
                                  const std::vector<int>& tag_ids, const std::string& search);
  
  // 查询用户卡片总数
  int GetCardCount(int user_id, const std::vector<int>& tag_ids, const std::string& search);
};
