#ifndef USER_REPOSITORY_H
#define USER_REPOSITORY_H

#include <memory>
#include <vector>
#include "../model/User.h"

namespace repository {

class UserRepository {
public:
    UserRepository() = default;
    ~UserRepository() = default;

    // 创建用户
    std::shared_ptr<model::User> createUser(const std::string& nickname);

    // 根据 ID 查询用户
    std::shared_ptr<model::User> getUserById(int id);

    // 根据昵称查询用户
    std::shared_ptr<model::User> getUserByNickname(const std::string& nickname);

    // 查询所有用户
    std::vector<std::shared_ptr<model::User>> getAllUsers();

    // 更新用户
    bool updateUser(const std::shared_ptr<model::User>& user);

    // 删除用户
    bool deleteUser(int id);

private:
    UserRepository(const UserRepository&) = delete;
    UserRepository& operator=(const UserRepository&) = delete;
};

} // namespace repository

#endif // USER_REPOSITORY_H
