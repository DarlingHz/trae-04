#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <memory>
#include <vector>
#include <string>
#include "../model/User.h"
#include "../repository/UserRepository.h"

namespace service {

class UserService {
public:
    UserService() : user_repository_(std::make_shared<repository::UserRepository>()) {}
    explicit UserService(std::shared_ptr<repository::UserRepository> user_repository)
        : user_repository_(std::move(user_repository)) {}
    ~UserService() = default;

    // 创建用户
    std::shared_ptr<model::User> createUser(const std::string& nickname);

    // 根据 ID 查询用户
    std::shared_ptr<model::User> getUserById(int id);

    // 根据昵称查询用户
    std::shared_ptr<model::User> getUserByNickname(const std::string& nickname);

    // 查询所有用户
    std::vector<std::shared_ptr<model::User>> getAllUsers();

    // 更新用户
    bool updateUser(int id, const std::string& nickname);

    // 删除用户
    bool deleteUser(int id);

private:
    std::shared_ptr<repository::UserRepository> user_repository_;

    UserService(const UserService&) = delete;
    UserService& operator=(const UserService&) = delete;
};

} // namespace service

#endif // USER_SERVICE_H
