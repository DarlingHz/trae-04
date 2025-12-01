#include "simple_jwt.h"
#include <iostream>
#include <algorithm>
#include <cstring>

SimpleJWT::SimpleJWT(const std::string& secret_key)
    : secret_key_(secret_key) {
}

SimpleJWT::~SimpleJWT() {
}

std::string SimpleJWT::GenerateToken(const std::map<std::string, std::string>& claims,
                                        const std::chrono::seconds& expiration) {
    // 创建头部
    std::string header = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";

    // 创建载荷
    std::ostringstream payload_stream;
    payload_stream << "{";

    // 添加过期时间
    auto now = std::chrono::system_clock::now();
    auto exp = now + expiration;
    auto exp_seconds = std::chrono::duration_cast<std::chrono::seconds>(exp.time_since_epoch()).count();
    payload_stream << "\"exp\":\"" << exp_seconds << "\",";

    // 添加其他声明
    for (auto it = claims.begin(); it != claims.end(); ++it) {
        if (it != claims.begin()) {
            payload_stream << ",";
        }
        payload_stream << "\"" << it->first << "\":\"" << it->second << "\"";
    }

    payload_stream << "}";
    std::string payload = payload_stream.str();

    // 编码头部和载荷
    std::string encoded_header = Base64Encode(header);
    std::string encoded_payload = Base64Encode(payload);

    // 创建签名
    std::string signature_data = encoded_header + "." + encoded_payload;
    std::string signature = Base64Encode(HMACSHA256(signature_data, secret_key_));

    // 组合令牌
    std::string token = encoded_header + "." + encoded_payload + "." + signature;

    return token;
}

bool SimpleJWT::VerifyToken(const std::string& token) {
    // 拆分令牌
    size_t first_dot = token.find(".");
    size_t second_dot = token.find(".", first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        return false;
    }

    std::string encoded_header = token.substr(0, first_dot);
    std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    std::string encoded_signature = token.substr(second_dot + 1);

    // 验证签名
    std::string signature_data = encoded_header + "." + encoded_payload;
    std::string expected_signature = Base64Encode(HMACSHA256(signature_data, secret_key_));

    if (encoded_signature != expected_signature) {
        return false;
    }

    // 验证过期时间
    std::string payload = Base64Decode(encoded_payload);
    size_t exp_pos = payload.find("\"exp\":\"");
    if (exp_pos == std::string::npos) {
        return false;
    }

    exp_pos += strlen("\"exp\":\"");
    size_t exp_end = payload.find("\"", exp_pos);
    if (exp_end == std::string::npos) {
        return false;
    }

    std::string exp_str = payload.substr(exp_pos, exp_end - exp_pos);
    long long exp_seconds = std::stoll(exp_str);

    auto now = std::chrono::system_clock::now();
    auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    if (now_seconds > exp_seconds) {
        return false;
    }

    return true;
}

std::map<std::string, std::string> SimpleJWT::GetClaims(const std::string& token) {
    std::map<std::string, std::string> claims;

    // 拆分令牌
    size_t first_dot = token.find(".");
    size_t second_dot = token.find(".", first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        return claims;
    }

    std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    std::string payload = Base64Decode(encoded_payload);

    // 解析载荷中的声明
    size_t pos = 0;
    while (pos < payload.size()) {
        // 找到键的开始
        size_t key_start = payload.find("\"", pos);
        if (key_start == std::string::npos) {
            break;
        }

        key_start += 1;
        size_t key_end = payload.find("\"", key_start);
        if (key_end == std::string::npos) {
            break;
        }

        std::string key = payload.substr(key_start, key_end - key_start);

        // 找到值的开始
        size_t value_start = payload.find("\"", key_end + 1);
        if (value_start == std::string::npos) {
            break;
        }

        value_start += 1;
        size_t value_end = payload.find("\"", value_start);
        if (value_end == std::string::npos) {
            break;
        }

        std::string value = payload.substr(value_start, value_end - value_start);

        claims[key] = value;

        pos = value_end + 1;
    }

    return claims;
}

std::string SimpleJWT::Base64Encode(const std::string& input) {
    BIO* bio = BIO_new(BIO_f_base64());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO* b64 = BIO_new_fp(stdout, BIO_NOCLOSE);
    b64 = BIO_push(bio, b64);

    BIO_write(b64, input.c_str(), input.size());
    BIO_flush(b64);

    char buffer[input.size() * 2];
    int len = BIO_read(b64, buffer, sizeof(buffer));

    BIO_free_all(b64);

    return std::string(buffer, len);
}

std::string SimpleJWT::Base64Decode(const std::string& input) {
    BIO* bio = BIO_new(BIO_f_base64());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO* b64 = BIO_new_mem_buf(input.c_str(), input.size());
    b64 = BIO_push(bio, b64);

    char buffer[input.size()];
    int len = BIO_read(b64, buffer, sizeof(buffer));

    BIO_free_all(b64);

    return std::string(buffer, len);
}

std::string SimpleJWT::HMACSHA256(const std::string& data, const std::string& key) {
    unsigned char* digest = HMAC(EVP_sha256(), key.c_str(), key.size(),
                                    reinterpret_cast<const unsigned char*>(data.c_str()), data.size(),
                                    nullptr, nullptr);

    char buffer[EVP_MAX_MD_SIZE * 2];
    for (int i = 0; i < EVP_MD_size(EVP_sha256()); ++i) {
        sprintf(buffer + i * 2, "%02x", digest[i]);
    }

    return std::string(buffer, EVP_MD_size(EVP_sha256()) * 2);
}
