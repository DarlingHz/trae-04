#pragma once

#include "dao/CategoryDAO.h"
#include "models/Category.h"
#include <vector>
#include <string>

namespace accounting {

class CategoryService {
public:
    CategoryService();
    ~CategoryService() = default;

    // 创建分类
    bool createCategory(const std::string& name, const std::string& type, Category& new_category);

    // 根据 ID 查询分类
    bool getCategoryById(int id, Category& category);

    // 查询所有分类
    std::vector<Category> getAllCategories(const std::string& type_filter = "");

    // 更新分类
    bool updateCategory(int id, const std::string& name, const std::string& type);

    // 删除分类
    bool deleteCategory(int id);

private:
    CategoryDAO category_dao_;

    // 验证分类数据
    bool validateCategoryData(const std::string& name, const std::string& type);
};

} // namespace accounting
