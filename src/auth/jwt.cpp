#include "jwt.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdexcept>

namespace auth {

JWT::JWT(const std::string& secret_key) : secret_key_(secret_key) {
    if (secret_key_.empty()) {
        throw std::invalid_argument("Secret key cannot be empty");
    }
}

std::string JWT::generateToken(const std::unordered_map<std::string, std::string>& payload, int expiry_hours) {
    // 创建头部
    std::string header = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
    
    // 创建载荷，添加过期时间
    long long exp = getCurrentTimestamp() + (expiry_hours * 3600);
    
    std::ostringstream payload_json;
    payload_json << "{";
    
    bool first = true;
    // 添加自定义载荷
    for (const auto& [key, value] : payload) {
        if (!first) {
            payload_json << ",";
        }
        payload_json << '"' << key << '":"' << value << '"';
        first = false;
    }
    
    // 添加过期时间
    if (!first) {
        payload_json << ",";
    }
    payload_json << '"' << "exp" << '":"' << exp << '"';
    payload_json << '}';
    
    // 编码头部和载荷
    std::string encoded_header = base64_encode(header);
    std::string encoded_payload = base64_encode(payload_json.str());
    
    // 创建签名数据
    std::string signature_data = encoded_header + "." + encoded_payload;
    
    // 生成签名
    std::string signature = hmac_sign(signature_data);
    
    // 组合成JWT
    return signature_data + "." + signature;
}

std::unordered_map<std::string, std::string> JWT::verifyAndParseToken(const std::string& token) {
    // 检查token格式
    size_t first_dot = token.find('.');
    size_t second_dot = token.rfind('.');
    
    if (first_dot == std::string::npos || second_dot == std::string::npos || first_dot == second_dot) {
        throw JWTException("Invalid token format");
    }
    
    // 提取各部分
    std::string encoded_header = token.substr(0, first_dot);
    std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    std::string signature = token.substr(second_dot + 1);
    
    // 验证签名
    std::string signature_data = encoded_header + "." + encoded_payload;
    if (!hmac_verify(signature_data, signature)) {
        throw JWTException("Invalid token signature");
    }
    
    // 解析载荷
    std::string payload_str = base64_decode(encoded_payload);
    
    // 简单JSON解析（实际应用中应使用专业JSON库）
    std::unordered_map<std::string, std::string> payload;
    
    // 移除前后的大括号
    size_t start = payload_str.find('{');
    size_t end = payload_str.rfind('}');
    
    if (start != std::string::npos && end != std::string::npos && start < end) {
        std::string content = payload_str.substr(start + 1, end - start - 1);
        
        size_t pos = 0;
        while (pos < content.size()) {
            // 查找键
            size_t key_start = content.find('"', pos);
            if (key_start == std::string::npos) break;
            
            size_t key_end = content.find('"', key_start + 1);
            if (key_end == std::string::npos) break;
            
            std::string key = content.substr(key_start + 1, key_end - key_start - 1);
            
            // 查找值
            size_t value_start = content.find('"', key_end + 1);
            if (value_start == std::string::npos) break;
            
            size_t value_end = content.find('"', value_start + 1);
            if (value_end == std::string::npos) break;
            
            std::string value = content.substr(value_start + 1, value_end - value_start - 1);
            
            payload[key] = value;
            
            pos = content.find(',', value_end);
            if (pos == std::string::npos) break;
            pos += 1;
        }
    }
    
    // 检查过期时间
    if (payload.find("exp") != payload.end()) {
        long long expiry = std::stoll(payload["exp"]);
        long long current = getCurrentTimestamp();
        
        if (current > expiry) {
            throw JWTException("Token has expired");
        }
    } else {
        throw JWTException("Token missing expiry time");
    }
    
    return payload;
}

bool JWT::isTokenExpired(const std::string& token) {
    try {
        auto payload = verifyAndParseToken(token);
        return false;
    } catch (const JWTException& e) {
        std::string msg = e.what();
        return msg.find("Token has expired") != std::string::npos;
    }
}

std::string JWT::base64_encode(const std::string& input) {
    // 使用OpenSSL进行base64编码
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    // 禁用换行符
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    // 写入数据
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    
    // 获取结果
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    
    std::string output(bufferPtr->data, bufferPtr->length);
    
    // 清理
    BIO_free_all(bio);
    BUF_MEM_free(bufferPtr);
    
    return output;
}

std::string JWT::base64_decode(const std::string& input) {
    // 填充修复
    std::string padded_input = input;
    size_t mod = padded_input.size() % 4;
    if (mod != 0) {
        padded_input.append(4 - mod, '=');
    }
    
    // 使用OpenSSL进行base64解码
    BIO *bio, *b64;
    char* buffer = nullptr;
    int buffer_length = 0;
    
    buffer_length = padded_input.size();
    buffer = (char*)malloc(buffer_length + 1);
    if (!buffer) {
        throw std::bad_alloc();
    }
    
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(padded_input.c_str(), -1);
    bio = BIO_push(b64, bio);
    
    int decode_length = BIO_read(bio, buffer, buffer_length);
    buffer[decode_length] = 0;
    
    std::string output(buffer, decode_length);
    
    // 清理
    free(buffer);
    BIO_free_all(bio);
    
    return output;
}

std::string JWT::hmac_sign(const std::string& data) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;
    
    // 使用OpenSSL生成HMAC-SHA256签名
    HMAC(EVP_sha256(), secret_key_.c_str(), secret_key_.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         digest, &digest_len);
    
    // 转换为base64
    std::string signature(reinterpret_cast<const char*>(digest), digest_len);
    return base64_encode(signature);
}

bool JWT::hmac_verify(const std::string& data, const std::string& signature) {
    // 重新计算签名
    std::string expected_signature = hmac_sign(data);
    
    // 比较签名
    // 注意：在生产环境中应使用时间恒定的比较函数以防止计时攻击
    return expected_signature == signature;
}

long long JWT::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    return seconds.count();
}

} // namespace auth
