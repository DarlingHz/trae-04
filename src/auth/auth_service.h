#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include "jwt.h"
#include "../models/user.h"
#include "../common/error.h"

namespace auth {

class AuthService {
public:
    AuthService();
    ~AuthService() = default;
    
    /**
     * 用户登录
     * @param username 用户名
     * @param password 密码
     * @return 登录成功返回token
     * @throws common::AppException 登录失败时抛出异常
     */
    std::string login(const std::string& username, const std::string& password);
    
    /**
     * 用户注册
     * @param username 用户名
     * @param password 密码
     * @return 注册成功返回用户ID
     * @throws common::AppException 注册失败时抛出异常
     */
    long long registerUser(const std::string& username, const std::string& password);
    
    /**
     * 验证token并获取用户信息
     * @param token JWT token
     * @return 用户信息
     * @throws common::AppException token无效或过期时抛出异常
     */
    std::shared_ptr<models::User> validateToken(const std::string& token);
    
    /**
     * 检查用户权限
     * @param userId 用户ID
     * @param requiredPermission 所需权限
     * @return 是否有权限
     */
    bool checkPermission(long long userId, const std::string& requiredPermission);

private:
    std::unique_ptr<JWT> m_jwt;
    
    /**
     * 生成用户token
     * @param user 用户信息
     * @return 生成的token
     */
    std::string generateToken(const models::User& user);
    
    /**
     * 验证密码
     * @param password 明文密码
     * @param passwordHash 存储的密码哈希
     * @return 密码是否匹配
     */
    bool verifyPassword(const std::string& password, const std::string& passwordHash);
};

// 全局认证服务实例
extern std::shared_ptr<AuthService> g_authService;

// 初始化认证服务
bool initAuthService();

} // namespace auth

#endif // AUTH_SERVICE_H
