#include "user_controller.h"
#include "../common/logger.h"
#include <chrono>

namespace controllers {

HttpResponse UserController::handleRequest(const HttpRequest& request) {
    try {
        // 根据路径和方法分发请求
        if (request.path == "/api/v1/users/register" && request.method == HttpMethod::POST) {
            return handleRegister(request);
        } else if (request.path == "/api/v1/users/login" && request.method == HttpMethod::POST) {
            return handleLogin(request);
        } else if (request.path == "/api/v1/users/me" && request.method == HttpMethod::GET) {
            return handleGetMe(request);
        }
        
        // 路径不匹配，返回404
        return createErrorResponse(common::ErrorCode::NOT_FOUND, "API endpoint not found");
    } catch (const common::AppException& e) {
        common::g_logger.error("User controller error: %s, code: %s", e.getMessage().c_str(), e.getCodeString().c_str());
        return createErrorResponse(e);
    } catch (const std::exception& e) {
        common::g_logger.error("User controller unexpected error: %s", e.what());
        return createErrorResponse(common::ErrorCode::INTERNAL_ERROR, "Internal server error");
    }
}

HttpResponse UserController::handleRegister(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 解析请求体
    common::JsonValue body;
    try {
        body = parseJson(request.body);
    } catch (const std::exception& e) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format");
    }
    
    // 验证必需字段
    if (!body.isObject() || !body["username"].isString() || body["username"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Username is required");
    }
    if (!body.isObject() || !body["password"].isString() || body["password"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Password is required");
    }
    
    // 获取用户名和密码
    std::string username = body["username"].asString();
    std::string password = body["password"].asString();
    
    // 调用认证服务进行注册
    long long userId = auth::g_authService->registerUser(username, password);
    
    // 构造响应
    common::JsonValue responseData;
    responseData["user_id"] = std::to_string(userId);
    responseData["username"] = username;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("User registered: username=%s, user_id=%lld, duration=%lldms", 
                     username.c_str(), userId, duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse UserController::handleLogin(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 解析请求体
    common::JsonValue body;
    try {
        body = parseJson(request.body);
    } catch (const std::exception& e) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format");
    }
    
    // 验证必需字段
    if (!body.isObject() || !body["username"].isString() || body["username"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Username is required");
    }
    if (!body.isObject() || !body["password"].isString() || body["password"].asString().empty()) {
        throw common::AppException(common::ErrorCode::INVALID_PARAM, "Password is required");
    }
    
    // 获取用户名和密码
    std::string username = body["username"].asString();
    std::string password = body["password"].asString();
    
    // 调用认证服务进行登录
    std::string token = auth::g_authService->login(username, password);
    
    // 构造响应
    common::JsonValue responseData;
    responseData["token"] = token;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("User logged in: username=%s, duration=%lldms", username.c_str(), duration);
    
    return createSuccessResponse(responseData);
}

HttpResponse UserController::handleGetMe(const HttpRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 验证认证token
    long long userId = authenticateRequest(request);
    
    // 获取用户信息
    auto user = models::g_userRepository->getUserById(userId);
    try {
        if (!user) {
            throw common::AppException(common::ErrorCode::USER_NOT_FOUND, "User not found");
        }
    } catch (const common::AppException& e) {
        if (e.getErrorCode() == common::ErrorCode::USER_NOT_FOUND) {
            throw common::AppException(common::ErrorCode::USER_NOT_FOUND, "User not found");
        }
        throw;
    }
    
    // 构造响应（不包含密码哈希）
    common::JsonValue responseData;
    responseData["id"] = std::to_string(user->getId());
    responseData["username"] = user->getUsername();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    common::g_logger.info("User profile retrieved: user_id=%lld, duration=%lldms", userId, duration);
    
    return createSuccessResponse(responseData);
}

long long UserController::authenticateRequest(const HttpRequest& request) {
    // 从请求头获取token
    std::string token = request.getAuthToken();
    if (token.empty()) {
        throw common::AppException(common::ErrorCode::UNAUTHORIZED, "Authorization token is required");
    }
    
    // 验证token并获取用户信息
    try {
        auto user = auth::g_authService->validateToken(token);
        if (!user) {
            throw common::AppException(common::ErrorCode::USER_NOT_FOUND, "User not found");
        }
        return user->getId();
    } catch (const common::AppException& e) {
        if (e.getErrorCode() == common::ErrorCode::INVALID_TOKEN) {
            throw common::AppException(common::ErrorCode::INVALID_TOKEN, "Invalid token");
        } else if (e.getErrorCode() == common::ErrorCode::TOKEN_EXPIRED) {
            throw common::AppException(common::ErrorCode::TOKEN_EXPIRED, "Token has expired");
        } else if (e.getErrorCode() == common::ErrorCode::FORBIDDEN) {
            throw common::AppException(common::ErrorCode::FORBIDDEN, "Permission denied");
        }
        throw; // Re-throw other exceptions
    }
}

} // namespace controllers
