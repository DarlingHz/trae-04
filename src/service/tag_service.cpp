#include "tag_service.h"
#include "util/time.h"

TagService::TagService(const std::string& db_path) : tag_dao_(db_path) {
}

Result<int> TagService::CreateTag(int user_id, const std::string& name) {
  // 验证标签名称是否为空
  if (name.empty()) {
    return Result<int>::Error("Tag name cannot be empty");
  }
  
  // 验证标签名称长度
  if (name.length() > 50) {
    return Result<int>::Error("Tag name cannot exceed 50 characters");
  }
  
  // 检查标签是否已存在
  auto existing_tag = tag_dao_.GetTagByName(user_id, name);
  if (existing_tag) {
    return Result<int>::Error("Tag already exists");
  }
  
  // 创建标签
  Tag tag;
  tag.user_id = user_id;
  tag.name = name;
  tag.created_at = TimeUtil::GetCurrentTimestamp();
  tag.updated_at = tag.created_at;
  
  int tag_id = tag_dao_.CreateTag(tag);
  if (tag_id == -1) {
    return Result<int>::Error("Failed to create tag");
  }
  
  return Result<int>::Success(tag_id);
}

Result<bool> TagService::UpdateTag(int user_id, int tag_id, const std::string& name) {
  // 验证标签名称是否为空
  if (name.empty()) {
    return Result<bool>::Error("Tag name cannot be empty");
  }
  
  // 验证标签名称长度
  if (name.length() > 50) {
    return Result<bool>::Error("Tag name cannot exceed 50 characters");
  }
  
  // 检查标签是否存在且属于该用户
  auto tag_result = GetTagById(user_id, tag_id);
  if (!tag_result.IsSuccess()) {
    return Result<bool>::Error(tag_result.GetError());
  }
  
  // 检查新标签名称是否已存在
  auto existing_tag = tag_dao_.GetTagByName(user_id, name);
  if (existing_tag && existing_tag->tag_id != tag_id) {
    return Result<bool>::Error("Tag name already exists");
  }
  
  // 更新标签
  Tag tag = tag_result.GetData();
  tag.name = name;
  tag.updated_at = TimeUtil::GetCurrentTimestamp();
  
  if (!tag_dao_.UpdateTag(tag)) {
    return Result<bool>::Error("Failed to update tag");
  }
  
  return Result<bool>::Success(true);
}

Result<bool> TagService::DeleteTag(int user_id, int tag_id) {
  // 检查标签是否存在且属于该用户
  auto tag_result = GetTagById(user_id, tag_id);
  if (!tag_result.IsSuccess()) {
    return Result<bool>::Error(tag_result.GetError());
  }
  
  // 删除标签
  if (!tag_dao_.DeleteTag(tag_id)) {
    return Result<bool>::Error("Failed to delete tag");
  }
  
  return Result<bool>::Success(true);
}

Result<Tag> TagService::GetTagById(int user_id, int tag_id) {
  // 获取标签
  auto tag = tag_dao_.GetTagById(tag_id);
  if (!tag) {
    return Result<Tag>::Error("Tag not found");
  }
  
  // 检查标签是否属于该用户
  if (tag->user_id != user_id) {
    return Result<Tag>::Error("Permission denied");
  }
  
  return Result<Tag>::Success(*tag);
}

Result<std::vector<TagWithCount>> TagService::GetTagList(int user_id) {
  // 获取标签列表
  std::vector<TagWithCount> tags = tag_dao_.GetTagList(user_id);
  
  return Result<std::vector<TagWithCount>>::Success(tags);
}
