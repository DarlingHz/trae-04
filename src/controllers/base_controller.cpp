#include "base_controller.h"
#include "../common/logger.h"
#include <sstream>
#include <regex>
#include <algorithm>

namespace controllers {

common::JsonValue BaseController::parseJson(const std::string& jsonStr) {
    // 简化的JSON解析实现
    // 实际生产环境中应使用专业的JSON库如nlohmann/json
    common::JsonValue result;
    
    try {
        // 移除前后的大括号
        std::string trimmed = jsonStr;
        // 去除前后空白
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
        
        if (trimmed.empty()) {
            return result;
        }
        
        // 确保字符串以{}包围
        if (trimmed.front() == '{' && trimmed.back() == '}') {
            trimmed = trimmed.substr(1, trimmed.size() - 2);
        } else {
            throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format: missing curly braces");
        }
        
        // 简单解析键值对
        size_t pos = 0;
        while (pos < trimmed.size()) {
            // 跳过空白
            pos = trimmed.find_first_not_of(" \t\n\r,", pos);
            if (pos == std::string::npos) break;
            
            // 查找键
            if (trimmed[pos] != '"') {
                throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format: key must be quoted");
            }
            pos++;
            size_t keyEnd = trimmed.find('"', pos);
            if (keyEnd == std::string::npos) {
                throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format: unterminated key");
            }
            std::string key = trimmed.substr(pos, keyEnd - pos);
            pos = keyEnd + 1;
            
            // 查找冒号
            pos = trimmed.find(':', pos);
            if (pos == std::string::npos) {
                throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format: missing colon");
            }
            pos++;
            
            // 查找值
            pos = trimmed.find_first_not_of(" \t\n\r", pos);
            if (pos == std::string::npos) {
                throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format: missing value");
            }
            
            std::string value;
            if (trimmed[pos] == '"') {
                // 字符串值
                pos++;
                size_t valueEnd = trimmed.find('"', pos);
                if (valueEnd == std::string::npos) {
                    throw common::AppException(common::ErrorCode::INVALID_PARAM, "Invalid JSON format: unterminated string value");
                }
                value = trimmed.substr(pos, valueEnd - pos);
                pos = valueEnd + 1;
            } else if (trimmed.substr(pos, 4) == "null") {
                // null值
                value = "null";
                pos += 4;
            } else if (trimmed.substr(pos, 4) == "true") {
                // true值
                value = "true";
                pos += 4;
            } else if (trimmed.substr(pos, 5) == "false") {
                // false值
                value = "false";
                pos += 5;
            } else {
                // 数字或其他值
                size_t valueEnd = trimmed.find_first_of(",}\r\n\t ", pos);
                if (valueEnd == std::string::npos) {
                    value = trimmed.substr(pos);
                    pos = trimmed.size();
                } else {
                    value = trimmed.substr(pos, valueEnd - pos);
                    pos = valueEnd;
                }
            }
            
            // 存储键值对
            result[key] = value;
        }
    } catch (const common::AppException&) {
        throw;
    } catch (const std::exception& e) {
                throw common::AppException(common::ErrorCode::INVALID_PARAM, std::string("Failed to parse JSON: ") + e.what());
            }
    
    return result;
}

std::string BaseController::toJson(const common::JsonValue& value) {
    return common::serializeJson(value);
}

HttpResponse BaseController::createSuccessResponse(const common::JsonValue& data) {
    common::JsonValue response;
    response["success"] = "true";
    response["data"] = toJson(data);
    
    HttpResponse httpResponse(200, toJson(response));
    return httpResponse;
}

HttpResponse BaseController::createErrorResponse(common::ErrorCode errorCode, const std::string& message) {
    common::JsonValue response;
    response["success"] = "false";
    response["code"] = common::ErrorMessage::getCodeString(errorCode);
    
    if (message.empty()) {
        response["message"] = common::ErrorMessage::getMessage(errorCode);
    } else {
        response["message"] = message;
    }
    
    // 设置适当的HTTP状态码
    int statusCode = 400;
    switch (errorCode) {
        case common::ErrorCode::SUCCESS:
            statusCode = 200;
            break;
        case common::ErrorCode::UNAUTHORIZED:
        case common::ErrorCode::TOKEN_EXPIRED:
        case common::ErrorCode::INVALID_TOKEN:
        case common::ErrorCode::USER_NOT_FOUND:
            statusCode = 401;
            break;
        case common::ErrorCode::FORBIDDEN:
            statusCode = 403;
            break;
        case common::ErrorCode::NOT_FOUND:
        case common::ErrorCode::PROBLEM_NOT_FOUND:
            statusCode = 404;
            break;
        case common::ErrorCode::USER_EXISTS:
            statusCode = 409;
            break;
        case common::ErrorCode::DATABASE_ERROR:
        case common::ErrorCode::INTERNAL_ERROR:
            statusCode = 500;
            break;
        default:
            statusCode = 400;
    }
    
    HttpResponse httpResponse(statusCode, toJson(response));
    return httpResponse;
}

HttpResponse BaseController::createErrorResponse(const common::AppException& exception) {
    return createErrorResponse(exception.getErrorCode(), exception.getMessage());
}

std::pair<int, int> BaseController::getPaginationParams(const HttpRequest& request, int defaultPageSize) {
    int page = 1;
    int pageSize = defaultPageSize;
    
    // 从查询参数获取page
    auto pageIt = request.queryParams.find("page");
    if (pageIt != request.queryParams.end()) {
        try {
            int parsedPage = std::stoi(pageIt->second);
            page = parsedPage > 0 ? parsedPage : 1;
        } catch (...) {
            // 解析失败，使用默认值
        }
    }
    
    // 从查询参数获取pageSize
    auto pageSizeIt = request.queryParams.find("page_size");
    if (pageSizeIt != request.queryParams.end()) {
        try {
            int parsedPageSize = std::stoi(pageSizeIt->second);
            // 限制pageSize范围
            pageSize = std::min(100, std::max(1, parsedPageSize));
        } catch (...) {
            // 解析失败，使用默认值
        }
    }
    
    return {page, pageSize};
}

} // namespace controllers
