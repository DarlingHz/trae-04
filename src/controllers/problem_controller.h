#ifndef PROBLEM_CONTROLLER_H
#define PROBLEM_CONTROLLER_H

#include "base_controller.h"
#include "../auth/auth_service.h"

namespace controllers {

class ProblemController : public BaseController {
public:
    ProblemController() = default;
    ~ProblemController() = default;
    
    /**
     * 处理HTTP请求
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleRequest(const HttpRequest& request) override;
    
private:
    /**
     * 处理创建题目
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleCreateProblem(const HttpRequest& request);
    
    /**
     * 处理获取题目详情
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleGetProblem(const HttpRequest& request);
    
    /**
     * 处理更新题目
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleUpdateProblem(const HttpRequest& request);
    
    /**
     * 处理删除题目（软删除）
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleDeleteProblem(const HttpRequest& request);
    
    /**
     * 处理题目列表查询
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleListProblems(const HttpRequest& request);
    
    /**
     * 处理题目搜索
     * @param request HTTP请求
     * @return HTTP响应
     */
    HttpResponse handleSearchProblems(const HttpRequest& request);
    
    /**
     * 验证请求中的认证token
     * @param request HTTP请求
     * @return 验证通过返回用户ID，否则抛出异常
     * @throws common::AppException 认证失败时抛出异常
     */
    long long authenticateRequest(const HttpRequest& request);
    
    /**
     * 从请求路径中提取题目ID
     * @param request HTTP请求
     * @return 题目ID
     * @throws common::AppException 无效的题目ID时抛出异常
     */
    long long extractProblemId(const HttpRequest& request);
};

} // namespace controllers

#endif // PROBLEM_CONTROLLER_H
