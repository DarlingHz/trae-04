#ifndef JWT_H
#define JWT_H

#include <string>
#include <chrono>
#include <unordered_map>

namespace auth {

// JWT相关异常类
class JWTException : public std::exception {
private:
    std::string message_;
public:
    explicit JWTException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
};

class JWT {
public:
    JWT(const std::string& secret_key);
    ~JWT() = default;
    
    /**
     * 生成JWT token
     * @param payload 载荷数据
     * @param expiry_hours 过期时间（小时）
     * @return 生成的JWT token
     */
    std::string generateToken(const std::unordered_map<std::string, std::string>& payload, int expiry_hours = 24);
    
    /**
     * 验证并解析JWT token
     * @param token JWT token字符串
     * @return 解析后的载荷数据
     * @throws JWTException 如果token无效或已过期
     */
    std::unordered_map<std::string, std::string> verifyAndParseToken(const std::string& token);
    
    /**
     * 检查token是否过期
     * @param token JWT token字符串
     * @return 如果token已过期返回true，否则返回false
     */
    bool isTokenExpired(const std::string& token);

private:
    // 编码base64
    std::string base64_encode(const std::string& input);
    
    // 解码base64
    std::string base64_decode(const std::string& input);
    
    // 生成HMAC签名
    std::string hmac_sign(const std::string& data);
    
    // 验证HMAC签名
    bool hmac_verify(const std::string& data, const std::string& signature);
    
    // 获取当前时间戳（秒）
    long long getCurrentTimestamp();
    
    std::string secret_key_;
};

} // namespace auth

#endif // JWT_H
