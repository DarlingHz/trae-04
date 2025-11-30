#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "../common/error.h"
#include "../common/json.h"

namespace controllers {

// HTTP方法枚举
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    HEAD
};

// HTTP请求结构体
struct HttpRequest {
    HttpMethod method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queryParams;
    std::map<std::string, std::string> pathParams;
    
    // 从headers中获取Authorization token
    std::string getAuthToken() const {
        auto it = headers.find("Authorization");
        if (it != headers.end()) {
            const std::string& authHeader = it->second;
            if (authHeader.substr(0, 7) == "Bearer ") {
                return authHeader.substr(7);
            }
        }
        return "";
    }
};

// HTTP响应结构体
struct HttpResponse {
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;
    
    HttpResponse(int code = 200, const std::string& responseBody = "")
        : statusCode(code), body(responseBody) {
        headers["Content-Type"] = "application/json";
    }
};

// 基础控制器类
class BaseController {
public:
    BaseController() = default;
    virtual ~BaseController() = default;
    
    /**
     * 处理HTTP请求
     * @param request HTTP请求
     * @return HTTP响应
     */
    virtual HttpResponse handleRequest(const HttpRequest& request) = 0;
    
    /**
     * 解析JSON请求体
     * @param jsonStr JSON字符串
     * @return 解析后的JSON对象
     * @throws common::AppException 解析失败时抛出异常
     */
    static common::JsonValue parseJson(const std::string& jsonStr);
    
    /**
     * 序列化对象为JSON字符串
     * @param value 要序列化的值
     * @return JSON字符串
     */
    static std::string toJson(const common::JsonValue& value);
    
    /**
     * 创建成功响应
     * @param data 响应数据
     * @return HTTP响应
     */
    static HttpResponse createSuccessResponse(const common::JsonValue& data = common::JsonValue());
    
    /**
     * 创建错误响应
     * @param errorCode 错误码
     * @param message 错误信息
     * @return HTTP响应
     */
    static HttpResponse createErrorResponse(common::ErrorCode errorCode, const std::string& message = "");
    
    /**
     * 创建错误响应
     * @param exception 异常对象
     * @return HTTP响应
     */
    static HttpResponse createErrorResponse(const common::AppException& exception);
    
    /**
     * 从请求中获取分页参数
     * @param request HTTP请求
     * @param defaultPageSize 默认每页大小
     * @return 分页参数（page, pageSize）
     */
    static std::pair<int, int> getPaginationParams(const HttpRequest& request, int defaultPageSize = 10);
};

} // namespace controllers

#endif // BASE_CONTROLLER_H
