#include "UserService.h"
#include "../utils/Logger.h"
#include <string>

namespace service {

std::shared_ptr<model::User> UserService::createUser(const std::string& nickname) {
    // 验证输入
    if (nickname.empty()) {
        LOG_ERROR("Nickname cannot be empty");
        return nullptr;
    }

    if (nickname.length() > 50) {
        LOG_ERROR("Nickname is too long (max 50 characters)");
        return nullptr;
    }

    // 检查昵称是否已存在
    auto existing_user = user_repository_->getUserByNickname(nickname);
    if (existing_user) {
        LOG_ERROR("Nickname already exists: " + nickname);
        return nullptr;
    }

    // 创建用户
    auto user = user_repository_->createUser(nickname);
    if (user) {
        LOG_INFO("User created successfully: " + std::to_string(user->getId()) + " - " + user->getNickname());
    } else {
        LOG_ERROR("Failed to create user with nickname: " + nickname);
    }

    return user;
}

std::shared_ptr<model::User> UserService::getUserById(int id) {
    if (id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(id));
        return nullptr;
    }

    auto user = user_repository_->getUserById(id);
    if (user) {
        LOG_DEBUG("User retrieved successfully: " + std::to_string(user->getId()) + " - " + user->getNickname());
    } else {
        LOG_DEBUG("User not found with ID: " + std::to_string(id));
    }

    return user;
}

std::shared_ptr<model::User> UserService::getUserByNickname(const std::string& nickname) {
    if (nickname.empty()) {
        LOG_ERROR("Nickname cannot be empty");
        return nullptr;
    }

    auto user = user_repository_->getUserByNickname(nickname);
    if (user) {
        LOG_DEBUG("User retrieved successfully by nickname: " + user->getNickname());
    } else {
        LOG_DEBUG("User not found with nickname: " + nickname);
    }

    return user;
}

std::vector<std::shared_ptr<model::User>> UserService::getAllUsers() {
    auto users = user_repository_->getAllUsers();
    LOG_DEBUG("Retrieved " + std::to_string(users.size()) + " users");
    return users;
}

bool UserService::updateUser(int id, const std::string& nickname) {
    if (id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(id));
        return false;
    }

    if (nickname.empty()) {
        LOG_ERROR("Nickname cannot be empty");
        return false;
    }

    if (nickname.length() > 50) {
        LOG_ERROR("Nickname is too long (max 50 characters)");
        return false;
    }

    // 检查用户是否存在
    auto user = user_repository_->getUserById(id);
    if (!user) {
        LOG_ERROR("User not found with ID: " + std::to_string(id));
        return false;
    }

    // 检查新昵称是否已被其他用户使用
    auto existing_user = user_repository_->getUserByNickname(nickname);
    if (existing_user && existing_user->getId() != id) {
        LOG_ERROR("Nickname already exists: " + nickname);
        return false;
    }

    // 更新用户信息
    user->setNickname(nickname);
    bool success = user_repository_->updateUser(user);
    if (success) {
        LOG_INFO("User updated successfully: " + std::to_string(id) + " - " + nickname);
    } else {
        LOG_ERROR("Failed to update user with ID: " + std::to_string(id));
    }

    return success;
}

bool UserService::deleteUser(int id) {
    if (id <= 0) {
        LOG_ERROR("Invalid user ID: " + std::to_string(id));
        return false;
    }

    // 检查用户是否存在
    auto user = user_repository_->getUserById(id);
    if (!user) {
        LOG_ERROR("User not found with ID: " + std::to_string(id));
        return false;
    }

    // 删除用户
    bool success = user_repository_->deleteUser(id);
    if (success) {
        LOG_INFO("User deleted successfully: " + std::to_string(id) + " - " + user->getNickname());
    } else {
        LOG_ERROR("Failed to delete user with ID: " + std::to_string(id));
    }

    return success;
}

} // namespace service
