#ifndef SIMPLE_JWT_H
#define SIMPLE_JWT_H

#include <string>
#include <map>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

class SimpleJWT {
public:
    SimpleJWT(const std::string& secret_key);
    ~SimpleJWT();

    // 生成JWT令牌
    std::string GenerateToken(const std::map<std::string, std::string>& claims,
                               const std::chrono::seconds& expiration = std::chrono::seconds(86400));

    // 验证JWT令牌
    bool VerifyToken(const std::string& token);

    // 从JWT令牌中获取声明
    std::map<std::string, std::string> GetClaims(const std::string& token);

private:
    std::string secret_key_;

    // Base64编码
    std::string Base64Encode(const std::string& input);

    // Base64解码
    std::string Base64Decode(const std::string& input);

    // HMAC SHA256签名
    std::string HMACSHA256(const std::string& data, const std::string& key);
};

#endif // SIMPLE_JWT_H
