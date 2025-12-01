#pragma once

#include "dao/tag_dao.h"
#include "result.h"
#include "model/tag.h"
#include <vector>

class TagService {
public:
  // 构造函数
  TagService(const std::string& db_path);
  
  // 创建标签
  Result<int> CreateTag(int user_id, const std::string& name);
  
  // 更新标签
  Result<bool> UpdateTag(int user_id, int tag_id, const std::string& name);
  
  // 删除标签
  Result<bool> DeleteTag(int user_id, int tag_id);
  
  // 获取标签详情
  Result<Tag> GetTagById(int user_id, int tag_id);
  
  // 获取标签列表
  Result<std::vector<TagWithCount>> GetTagList(int user_id);
  
private:
  TagDao tag_dao_;
};
