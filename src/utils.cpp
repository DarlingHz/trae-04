// 工具函数源文件

#include "utils.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <random>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <map>

// 字符串工具函数
namespace string_utils {

std::string Trim(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    
    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
        end--;
    }
    
    return str.substr(start, end - start + 1);
}

std::string TrimLeft(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    
    return str.substr(start);
}

std::string TrimRight(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    
    size_t end = str.size() - 1;
    while (end > 0 && std::isspace(static_cast<unsigned char>(str[end]))) {
        end--;
    }
    
    return str.substr(0, end + 1);
}

std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool IsEmptyOrWhitespace(const std::string& str) {
    return str.empty() || std::all_of(str.begin(), str.end(), 
                                         [](unsigned char c) { return std::isspace(c); });
}

bool IsValidEmail(const std::string& email) {
    if (email.empty()) {
        return false;
    }
    
    size_t at_pos = email.find('@');
    if (at_pos == std::string::npos || at_pos == 0 || at_pos == email.size() - 1) {
        return false;
    }
    
    size_t dot_pos = email.find('.', at_pos);
    if (dot_pos == std::string::npos || dot_pos == at_pos + 1 || dot_pos == email.size() - 1) {
        return false;
    }
    
    return true;
}

std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    // 如果最后一个字符是分隔符，添加一个空字符串
    if (!str.empty() && str.back() == delimiter) {
        result.push_back("");
    }
    
    return result;
}

std::string Join(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) {
        return "";
    }
    
    std::stringstream ss;
    ss << parts[0];
    
    for (size_t i = 1; i < parts.size(); ++i) {
        ss << separator << parts[i];
    }
    
    return ss.str();
}

}

// 时间工具函数
namespace time_utils {

std::string ToIsoString(const std::chrono::system_clock::time_point& tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&t);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    
    return ss.str();
}

std::optional<std::chrono::system_clock::time_point> FromIsoString(const std::string& str) {
    try {
        std::tm tm = {};
        std::stringstream ss(str);
        
        // 尝试解析ISO 8601格式
        if (str.find('T') != std::string::npos) {
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        } else {
            // 尝试解析日期格式
            ss >> std::get_time(&tm, "%Y-%m-%d");
        }
        
        if (ss.fail()) {
            return std::nullopt;
        }
        
        std::time_t t = std::mktime(&tm);
        if (t == -1) {
            return std::nullopt;
        }
        
        return std::chrono::system_clock::from_time_t(t);
        
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::chrono::system_clock::time_point Now() {
    return std::chrono::system_clock::now();
}

std::time_t ToUnixTimestamp(const std::chrono::system_clock::time_point& time_point) {
    return std::chrono::system_clock::to_time_t(time_point);
}

std::chrono::system_clock::time_point FromUnixTimestamp(std::time_t timestamp) {
    return std::chrono::system_clock::from_time_t(timestamp);
}

}

// 加密工具函数
namespace crypto_utils {

std::string GenerateRandomString(int length) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.size() - 1);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += chars[distribution(generator)];
    }
    
    return result;
}

std::string Sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

}

// URL工具函数
namespace url_utils {

std::string UrlEncode(const std::string& str) {
    std::string result;
    result.reserve(str.size() * 3);
    
    for (char c : str) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else {
            result += '%';
            result += std::toupper(static_cast<unsigned char>(c >> 4));
            result += std::toupper(static_cast<unsigned char>(c & 0x0F));
        }
    }
    
    return result;
}

std::string UrlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 < str.size()) {
                char hex1 = str[i + 1];
                char hex2 = str[i + 2];
                
                int value = 0;
                if (std::isdigit(static_cast<unsigned char>(hex1))) {
                    value += (hex1 - '0') * 16;
                } else if (std::isupper(static_cast<unsigned char>(hex1))) {
                    value += (hex1 - 'A' + 10) * 16;
                } else if (std::islower(static_cast<unsigned char>(hex1))) {
                    value += (hex1 - 'a' + 10) * 16;
                }
                
                if (std::isdigit(static_cast<unsigned char>(hex2))) {
                    value += (hex2 - '0');
                } else if (std::isupper(static_cast<unsigned char>(hex2))) {
                    value += (hex2 - 'A' + 10);
                } else if (std::islower(static_cast<unsigned char>(hex2))) {
                    value += (hex2 - 'a' + 10);
                }
                
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    
    return result;
}

std::unordered_map<std::string, std::string> ParseQueryParams(const std::string& query_string) {
    std::unordered_map<std::string, std::string> params;
    
    std::vector<std::string> parts = string_utils::Split(query_string, '&');
    for (const auto& part : parts) {
        std::vector<std::string> key_value = string_utils::Split(part, '=');
        if (key_value.size() >= 1) {
            std::string key = UrlDecode(key_value[0]);
            std::string value;
            if (key_value.size() >= 2) {
                value = UrlDecode(key_value[1]);
            }
            params[key] = value;
        }
    }
    
    return params;
}

} // namespace url_utils