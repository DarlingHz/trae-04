#ifndef RECORD_CONTROLLER_H
#define RECORD_CONTROLLER_H

#include "base_controller.h"
#include "../auth/auth_service.h"

namespace controllers {

class RecordController : public BaseController {
public:
    RecordController() = default;
    ~RecordController() = default;
    
    /**
     * 处理HTTP请求
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleRequest(const HttpRequest& request) override;
    
private:
    /**
     * 处理创建刷题记录
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleCreateRecord(const HttpRequest& request);
    
    /**
     * 处理查询用户刷题记录
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleGetUserRecords(const HttpRequest& request);
    
    /**
     * 处理获取用户统计信息
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleGetUserStats(const HttpRequest& request);
    
    /**
     * 验证请求中的认证token
     * @param request HTTP请求
     * @return 验证通过返回用户ID，否则抛出异常
     * @throws common::AppException 认证失败时抛出异常
     */
    long long authenticateRequest(const HttpRequest& request);
    
    /**
     * 从请求路径中提取问题ID
     * @param request HTTP请求
     * @return 问题ID
     * @throws common::AppException 无效的问题ID时抛出异常
     */
    long long extractProblemId(const HttpRequest& request);
    
    /**
     * 从请求路径中提取用户ID
     * @param request HTTP请求
     * @return 用户ID
     * @throws common::AppException 无效的用户ID时抛出异常
     */
    long long extractUserId(const HttpRequest& request);
};

} // namespace controllers

#endif // RECORD_CONTROLLER_H
