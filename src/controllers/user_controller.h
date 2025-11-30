#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "base_controller.h"
#include "../auth/auth_service.h"

namespace controllers {

class UserController : public BaseController {
public:
    UserController() = default;
    ~UserController() = default;
    
    /**
     * 处理HTTP请求
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleRequest(const HttpRequest& request) override;
    
private:
    /**
     * 处理用户注册
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleRegister(const HttpRequest& request);
    
    /**
     * 处理用户登录
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleLogin(const HttpRequest& request);
    
    /**
     * 处理获取当前用户信息
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleGetMe(const HttpRequest& request);
    
    /**
     * 验证请求中的认证token
     * @param request HTTP请求
     * @return 验证通过返回用户ID，否则抛出异常
     * @throws common::AppException 认证失败时抛出异常
     */
    long long authenticateRequest(const HttpRequest& request);
};

} // namespace controllers

#endif // USER_CONTROLLER_H
