#include "CategoryService.h"
#include <iostream>
#include <algorithm>

namespace accounting {

CategoryService::CategoryService() {
}

bool CategoryService::createCategory(const std::string& name, const std::string& type, Category& new_category) {
    // 验证分类数据
    if (!validateCategoryData(name, type)) {
        std::cerr << "Invalid category data." << std::endl;
        return false;
    }

    // 创建分类对象
    Category category(0, name, type);

    // 调用 DAO 创建分类
    int new_category_id = 0;
    if (!category_dao_.createCategory(category, new_category_id)) {
        std::cerr << "Failed to create category." << std::endl;
        return false;
    }

    // 获取新创建的分类信息
    if (!category_dao_.getCategoryById(new_category_id, new_category)) {
        std::cerr << "Failed to get new category." << std::endl;
        return false;
    }

    return true;
}

bool CategoryService::getCategoryById(int id, Category& category) {
    if (id <= 0) {
        std::cerr << "Invalid category ID." << std::endl;
        return false;
    }

    return category_dao_.getCategoryById(id, category);
}

std::vector<Category> CategoryService::getAllCategories(const std::string& type_filter) {
    // 验证类型过滤条件
    if (!type_filter.empty()) {
        std::vector<std::string> valid_types = {"income", "expense"};
        if (std::find(valid_types.begin(), valid_types.end(), type_filter) == valid_types.end()) {
            std::cerr << "Invalid category type filter." << std::endl;
            return std::vector<Category>();
        }
    }

    return category_dao_.getAllCategories(type_filter);
}

bool CategoryService::updateCategory(int id, const std::string& name, const std::string& type) {
    if (id <= 0) {
        std::cerr << "Invalid category ID." << std::endl;
        return false;
    }

    // 验证分类数据
    if (!validateCategoryData(name, type)) {
        std::cerr << "Invalid category data." << std::endl;
        return false;
    }

    // 检查分类是否存在
    Category existing_category;
    if (!category_dao_.getCategoryById(id, existing_category)) {
        std::cerr << "Category not found." << std::endl;
        return false;
    }

    // 更新分类信息
    existing_category.setName(name);
    existing_category.setType(type);

    if (!category_dao_.updateCategory(existing_category)) {
        std::cerr << "Failed to update category." << std::endl;
        return false;
    }

    return true;
}

bool CategoryService::deleteCategory(int id) {
    if (id <= 0) {
        std::cerr << "Invalid category ID." << std::endl;
        return false;
    }

    // 检查分类是否存在
    Category existing_category;
    if (!category_dao_.getCategoryById(id, existing_category)) {
        std::cerr << "Category not found." << std::endl;
        return false;
    }

    // 调用 DAO 删除分类
    if (!category_dao_.deleteCategory(id)) {
        std::cerr << "Failed to delete category." << std::endl;
        return false;
    }

    return true;
}

bool CategoryService::validateCategoryData(const std::string& name, const std::string& type) {
    // 验证分类名称是否为空
    if (name.empty()) {
        std::cerr << "Category name cannot be empty." << std::endl;
        return false;
    }

    // 验证分类名称长度是否合法
    if (name.length() > 50) {
        std::cerr << "Category name cannot exceed 50 characters." << std::endl;
        return false;
    }

    // 验证分类类型是否合法
    std::vector<std::string> valid_types = {"income", "expense"};
    if (std::find(valid_types.begin(), valid_types.end(), type) == valid_types.end()) {
        std::cerr << "Invalid category type." << std::endl;
        return false;
    }

    return true;
}

} // namespace accounting
