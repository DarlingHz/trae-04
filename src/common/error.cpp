#include "error.h"
#include <sstream>

namespace common {

// 初始化错误信息映射
std::unordered_map<ErrorCode, std::string> ErrorMessage::codeToMessage = {
    {ErrorCode::SUCCESS, "Success"},
    {ErrorCode::INVALID_PARAM, "Invalid parameter"},
    {ErrorCode::INTERNAL_ERROR, "Internal server error"},
    {ErrorCode::NOT_FOUND, "Resource not found"},
    {ErrorCode::USER_EXISTS, "User already exists"},
    {ErrorCode::INVALID_CREDENTIALS, "Invalid username or password"},
    {ErrorCode::USER_NOT_FOUND, "User not found"},
    {ErrorCode::UNAUTHORIZED, "Unauthorized"},
    {ErrorCode::TOKEN_EXPIRED, "Token expired"},
    {ErrorCode::INVALID_TOKEN, "Invalid token"},
    {ErrorCode::FORBIDDEN, "Access forbidden"},
    {ErrorCode::PROBLEM_NOT_FOUND, "Problem not found"},
    {ErrorCode::DATABASE_ERROR, "Database error"},
    {ErrorCode::CONNECTION_POOL_EXHAUSTED, "Connection pool exhausted"},
};

std::unordered_map<ErrorCode, std::string> ErrorMessage::codeToString = {
    {ErrorCode::SUCCESS, "SUCCESS"},
    {ErrorCode::INVALID_PARAM, "INVALID_PARAM"},
    {ErrorCode::INTERNAL_ERROR, "INTERNAL_ERROR"},
    {ErrorCode::NOT_FOUND, "NOT_FOUND"},
    {ErrorCode::USER_EXISTS, "USER_EXISTS"},
    {ErrorCode::INVALID_CREDENTIALS, "INVALID_CREDENTIALS"},
    {ErrorCode::USER_NOT_FOUND, "USER_NOT_FOUND"},
    {ErrorCode::UNAUTHORIZED, "UNAUTHORIZED"},
    {ErrorCode::TOKEN_EXPIRED, "TOKEN_EXPIRED"},
    {ErrorCode::INVALID_TOKEN, "INVALID_TOKEN"},
    {ErrorCode::FORBIDDEN, "FORBIDDEN"},
    {ErrorCode::PROBLEM_NOT_FOUND, "PROBLEM_NOT_FOUND"},
    {ErrorCode::DATABASE_ERROR, "DATABASE_ERROR"},
    {ErrorCode::CONNECTION_POOL_EXHAUSTED, "CONNECTION_POOL_EXHAUSTED"},
};

std::string ErrorMessage::getMessage(ErrorCode code) {
    auto it = codeToMessage.find(code);
    if (it != codeToMessage.end()) {
        return it->second;
    }
    return "Unknown error";
}

std::string ErrorMessage::getCodeString(ErrorCode code) {
    auto it = codeToString.find(code);
    if (it != codeToString.end()) {
        return it->second;
    }
    return "UNKNOWN_ERROR";
}

AppException::AppException(ErrorCode code, const std::string& message) 
    : code_(code), message_(message) {
    std::stringstream ss;
    ss << "Error [" << ErrorMessage::getCodeString(code) << "]: " 
       << (message.empty() ? ErrorMessage::getMessage(code) : message);
    fullMessage_ = ss.str();
}

const char* AppException::what() const noexcept {
    return fullMessage_.c_str();
}

ErrorCode AppException::getErrorCode() const {
    return code_;
}

std::string AppException::getMessage() const {
    return message_.empty() ? ErrorMessage::getMessage(code_) : message_;
}

std::string AppException::getCodeString() const {
    return ErrorMessage::getCodeString(code_);
}

Response::Response() : code_(ErrorCode::SUCCESS) {
}

Response::Response(ErrorCode code, const std::string& message) 
    : code_(code), message_(message) {
}

void Response::setSuccess(const std::string& data) {
    code_ = ErrorCode::SUCCESS;
    message_ = "Success";
    data_ = data;
}

void Response::setError(ErrorCode code, const std::string& message) {
    code_ = code;
    message_ = message.empty() ? ErrorMessage::getMessage(code) : message;
    data_ = "";
}

std::string Response::toJson() const {
    std::stringstream ss;
    ss << "{";
    
    if (code_ == ErrorCode::SUCCESS) {
        // 成功响应格式
        ss << "\"code\": \"SUCCESS\", \"message\": \"Success\"";
        if (!data_.empty()) {
            ss << ", \"data\": " << data_;
        }
    } else {
        // 错误响应格式
        ss << "\"code\": \"" << ErrorMessage::getCodeString(code_) << "\", "
           << "\"message\": \"" << message_ << "\"";
    }
    
    ss << "}";
    return ss.str();
}

ErrorCode Response::getCode() const {
    return code_;
}

std::string Response::getMessage() const {
    return message_;
}

std::string Response::getData() const {
    return data_;
}

void Response::setData(const std::string& data) {
    data_ = data;
}

} // namespace common
