#ifndef USER_H
#define USER_H

#include <string>
#include <memory>

namespace models {

// 用户实体类
class User {
public:
    User();
    User(long long id, const std::string& username, const std::string& passwordHash);
    
    // Getters and setters
    long long getId() const;
    void setId(long long id);
    
    const std::string& getUsername() const;
    void setUsername(const std::string& username);
    
    const std::string& getPasswordHash() const;
    void setPasswordHash(const std::string& passwordHash);
    
    // 检查密码
    bool checkPassword(const std::string& password) const;
    
    // 生成密码哈希
    static std::string hashPassword(const std::string& password);

private:
    long long id_;
    std::string username_;
    std::string passwordHash_;
};

// 用户数据访问接口
class UserRepository {
public:
    virtual ~UserRepository() = default;
    
    // 创建用户
    virtual bool createUser(const User& user) = 0;
    
    // 根据ID获取用户
    virtual std::shared_ptr<User> getUserById(long long id) = 0;
    
    // 根据用户名获取用户
    virtual std::shared_ptr<User> getUserByUsername(const std::string& username) = 0;
    
    // 更新用户信息
    virtual bool updateUser(const User& user) = 0;
    
    // 检查用户名是否存在
    virtual bool usernameExists(const std::string& username) = 0;
};

// 用户数据访问实现（基于数据库）
class DatabaseUserRepository : public UserRepository {
public:
    DatabaseUserRepository();
    ~DatabaseUserRepository() override;
    
    bool createUser(const User& user) override;
    std::shared_ptr<User> getUserById(long long id) override;
    std::shared_ptr<User> getUserByUsername(const std::string& username) override;
    bool updateUser(const User& user) override;
    bool usernameExists(const std::string& username) override;
};

// 全局用户仓库实例
extern std::shared_ptr<UserRepository> g_userRepository;

// 初始化用户仓库
bool initUserRepository();

} // namespace models

#endif // USER_H
