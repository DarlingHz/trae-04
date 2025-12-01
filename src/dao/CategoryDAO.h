#pragma once

#include "models/Category.h"
#include <vector>

namespace accounting {

class CategoryDAO {
public:
    // 创建分类
    bool createCategory(const Category& category, int& new_category_id);

    // 根据 ID 查询分类
    bool getCategoryById(int id, Category& category);

    // 查询所有分类
    std::vector<Category> getAllCategories(const std::string& type_filter = "");

    // 更新分类
    bool updateCategory(const Category& category);

    // 删除分类
    bool deleteCategory(int id);
};

} // namespace accounting
