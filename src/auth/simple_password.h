#ifndef SIMPLE_PASSWORD_H
#define SIMPLE_PASSWORD_H

#include <string>
#include <map>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

class SimplePassword {
public:
    SimplePassword();
    ~SimplePassword();

    // 生成密码哈希和盐
    std::pair<std::string, std::string> GeneratePasswordHash(const std::string& password);

    // 验证密码
    bool VerifyPassword(const std::string& password, const std::string& hash, const std::string& salt);

private:
    // Base64编码
    std::string Base64Encode(const std::string& input);

    // Base64解码
    std::string Base64Decode(const std::string& input);

    // SHA256哈希
    std::string SHA256(const std::string& data);

    // 生成随机盐
    std::string GenerateSalt(size_t length = 16);
};

#endif // SIMPLE_PASSWORD_H
