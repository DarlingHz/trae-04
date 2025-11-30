#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <unordered_map>

namespace common {

// 错误码枚举
enum class ErrorCode {
    SUCCESS = 0,
    
    // 通用错误
    INVALID_PARAM = 1001,
    INTERNAL_ERROR = 1002,
    NOT_FOUND = 1003,
    
    // 用户相关错误
    USER_EXISTS = 2001,
    INVALID_CREDENTIALS = 2002,
    USER_NOT_FOUND = 2003,
    
    // 认证相关错误
    UNAUTHORIZED = 3001,
    TOKEN_EXPIRED = 3002,
    INVALID_TOKEN = 3003,
    
    // 权限相关错误
    FORBIDDEN = 4001,
    
    // 题目相关错误
    PROBLEM_NOT_FOUND = 5001,
    
    // 数据库相关错误
    DATABASE_ERROR = 6001,
    CONNECTION_POOL_EXHAUSTED = 6002,
};

// 错误信息映射
class ErrorMessage {
public:
    static std::string getMessage(ErrorCode code);
    static std::string getCodeString(ErrorCode code);

private:
    static std::unordered_map<ErrorCode, std::string> codeToMessage;
    static std::unordered_map<ErrorCode, std::string> codeToString;
};

// 自定义异常类
class AppException : public std::exception {
public:
    AppException(ErrorCode code, const std::string& message = "");
    const char* what() const noexcept override;
    ErrorCode getErrorCode() const;
    std::string getMessage() const;
    std::string getCodeString() const;

private:
    ErrorCode code_;
    std::string message_;
    std::string fullMessage_;
};

// 响应结果类
class Response {
public:
    Response();
    Response(ErrorCode code, const std::string& message = "");
    
    // 设置成功响应
    void setSuccess(const std::string& data = "");
    
    // 设置错误响应
    void setError(ErrorCode code, const std::string& message = "");
    
    // 转换为JSON字符串
    std::string toJson() const;
    
    // 获取状态码
    ErrorCode getCode() const;
    
    // 获取消息
    std::string getMessage() const;
    
    // 获取数据
    std::string getData() const;
    
    // 设置数据
    void setData(const std::string& data);

private:
    ErrorCode code_;
    std::string message_;
    std::string data_;
};

} // namespace common

#endif // ERROR_H
