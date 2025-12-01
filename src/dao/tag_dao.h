#pragma once

#include "base_dao.h"
#include "model/tag.h"
#include <optional>
#include <vector>

class TagDao : public BaseDao {
public:
  // 构造函数
  TagDao(const std::string& db_path);
  
  // 创建标签表
  void CreateTagTable();
  
  // 创建标签
  int CreateTag(const Tag& tag);
  
  // 更新标签
  bool UpdateTag(const Tag& tag);
  
  // 删除标签
  bool DeleteTag(int tag_id);
  
  // 根据标签ID查询标签
  std::optional<Tag> GetTagById(int tag_id);
  
  // 根据标签名称查询标签
  std::optional<Tag> GetTagByName(int user_id, const std::string& name);
  
  // 查询用户标签列表
  std::vector<TagWithCount> GetTagList(int user_id);
};
