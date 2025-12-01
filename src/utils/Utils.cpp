#include "utils/Utils.hpp"
#include <algorithm>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace Utils {

    // 时间处理
    std::string getCurrentTimeStr() {
        time_t now = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return std::string(buf);
    }

    time_t getCurrentTime() {
        return time(nullptr);
    }

    std::string formatTime(time_t t, const std::string& format) {
        char buf[100];
        strftime(buf, sizeof(buf), format.c_str(), localtime(&t));
        return std::string(buf);
    }

    time_t parseTime(const std::string& time_str, const std::string& format) {
        struct tm tm = {};
        std::istringstream ss(time_str);
        ss >> std::get_time(&tm, format.c_str());
        return mktime(&tm);
    }

    bool isToday(time_t t) {
        time_t now = time(nullptr);
        struct tm tm_now = {};
        struct tm tm_t = {};
        localtime_r(&now, &tm_now);
        localtime_r(&t, &tm_t);
        return (tm_now.tm_year == tm_t.tm_year && tm_now.tm_yday == tm_t.tm_yday);
    }

    bool isThisMinute(time_t t) {
        time_t now = time(nullptr);
        struct tm tm_now = {};
        struct tm tm_t = {};
        localtime_r(&now, &tm_now);
        localtime_r(&t, &tm_t);
        return (tm_now.tm_year == tm_t.tm_year && tm_now.tm_mon == tm_t.tm_mon &&
                tm_now.tm_mday == tm_t.tm_mday && tm_now.tm_hour == tm_t.tm_hour &&
                tm_now.tm_min == tm_t.tm_min);
    }

    // 随机数生成
    std::string generateRandomString(size_t length) {
        static const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
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

    uint64_t generateRandomUint64() {
        std::random_device rd;
        std::mt19937_64 generator(rd());
        return generator();
    }

    // JSON 实现
    bool JsonValue::hasKey(const std::string& key) const {
        if (type_ != OBJECT) {
            return false;
        }
        return object_value_.find(key) != object_value_.end();
    }

    JsonValue& JsonValue::operator[](const std::string& key) {
        if (type_ != OBJECT) {
            type_ = OBJECT;
            object_value_.clear();
        }
        return object_value_[key];
    }

    const JsonValue& JsonValue::operator[](const std::string& key) const {
        static JsonValue null_value;
        if (type_ != OBJECT) {
            return null_value;
        }
        auto it = object_value_.find(key);
        if (it == object_value_.end()) {
            return null_value;
        }
        return it->second;
    }

    std::string JsonValue::toString() const {
        switch (type_) {
            case NULL_TYPE:
                return "null";
            case BOOLEAN:
                return bool_value_ ? "true" : "false";
            case NUMBER:
                return std::to_string(double_value_);
            case STRING:
                return "\"" + string_value_ + "\"";
            case ARRAY:
                {
                    std::string result = "[";
                    for (size_t i = 0; i < array_value_.size(); ++i) {
                        if (i > 0) {
                            result += ",";
                        }
                        result += array_value_[i].toString();
                    }
                    result += "]";
                    return result;
                }
            case OBJECT:
                {
                    std::string result = "{";
                    size_t i = 0;
                    for (const auto& pair : object_value_) {
                        if (i > 0) {
                            result += ",";
                        }
                        result += "\"" + pair.first + "\":" + pair.second.toString();
                        ++i;
                    }
                    result += "}";
                    return result;
                }
            default:
                return "null";
        }
    }

    JsonValue parseJson(const std::string& json_str) {
        // 简化版 JSON 解析，仅支持基本类型
        JsonValue result;
        std::string trimmed = trim(json_str);

        if (trimmed.empty()) {
            return result;
        }

        if (trimmed == "null") {
            return result;
        }

        if (trimmed == "true") {
            return JsonValue(true);
        }

        if (trimmed == "false") {
            return JsonValue(false);
        }

        if (trimmed.front() == '"' && trimmed.back() == '"') {
            std::string str = trimmed.substr(1, trimmed.size() - 2);
            // 简单处理转义字符
            std::string unescaped;
            for (size_t i = 0; i < str.size(); ++i) {
                if (str[i] == '\\' && i + 1 < str.size()) {
                    switch (str[i + 1]) {
                        case '"':
                            unescaped += '"';
                            break;
                        case '\\':
                            unescaped += '\\';
                            break;
                        case 'n':
                            unescaped += '\n';
                            break;
                        case 'r':
                            unescaped += '\r';
                            break;
                        case 't':
                            unescaped += '\t';
                            break;
                        default:
                            unescaped += str[i + 1];
                            break;
                    }
                    ++i;
                } else {
                    unescaped += str[i];
                }
            }
            return JsonValue(unescaped);
        }

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            JsonValue array(JsonValue::ARRAY);
            std::string elements_str = trimmed.substr(1, trimmed.size() - 2);
            // 简单分割数组元素
            std::vector<std::string> elements;
            int depth = 0;
            std::string current_element;
            for (char c : elements_str) {
                if (c == '[' || c == '{') {
                    depth++;
                } else if (c == ']' || c == '}') {
                    depth--;
                } else if (c == ',' && depth == 0) {
                    elements.push_back(trim(current_element));
                    current_element.clear();
                    continue;
                }
                current_element += c;
            }
            if (!current_element.empty()) {
                elements.push_back(trim(current_element));
            }

            for (const std::string& element_str : elements) {
                array.asArray().push_back(parseJson(element_str));
            }
            return array;
        }

        if (trimmed.front() == '{' && trimmed.back() == '}') {
            JsonValue object(JsonValue::OBJECT);
            std::string members_str = trimmed.substr(1, trimmed.size() - 2);
            // 简单分割对象成员
            std::vector<std::string> members;
            int depth = 0;
            std::string current_member;
            for (char c : members_str) {
                if (c == '[' || c == '{') {
                    depth++;
                } else if (c == ']' || c == '}') {
                    depth--;
                } else if (c == ',' && depth == 0) {
                    members.push_back(trim(current_member));
                    current_member.clear();
                    continue;
                }
                current_member += c;
            }
            if (!current_member.empty()) {
                members.push_back(trim(current_member));
            }

            for (const std::string& member_str : members) {
                size_t colon_pos = member_str.find(':');
                if (colon_pos == std::string::npos) {
                    continue;
                }
                std::string key_str = trim(member_str.substr(0, colon_pos));
                std::string value_str = trim(member_str.substr(colon_pos + 1));

                if (key_str.front() == '"' && key_str.back() == '"') {
                    std::string key = key_str.substr(1, key_str.size() - 2);
                    object[key] = parseJson(value_str);
                }
            }
            return object;
        }

        // 尝试解析为数字
        try {
            size_t pos;
            double num = std::stod(trimmed, &pos);
            if (pos == trimmed.size()) {
                return JsonValue(num);
            }
        } catch (...) {
            // 解析失败，返回 null
        }

        return result;
    }

    std::string jsonToString(const JsonValue& json) {
        return json.toString();
    }

    // 字符串处理
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream token_stream(str);
        while (std::getline(token_stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string trim(const std::string& str) {
        auto start = str.begin();
        while (start != str.end() && std::isspace(*start)) {
            start++;
        }

        auto end = str.end();
        do {
            end--;
        } while (std::distance(start, end) > 0 && std::isspace(*end));

        return std::string(start, end + 1);
    }

    bool startsWith(const std::string& str, const std::string& prefix) {
        if (prefix.size() > str.size()) {
            return false;
        }
        return std::equal(prefix.begin(), prefix.end(), str.begin());
    }

    bool endsWith(const std::string& str, const std::string& suffix) {
        if (suffix.size() > str.size()) {
            return false;
        }
        return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
    }

    // Base64 编码
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string base64Encode(const std::string& input) {
        std::string result;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        for (char c : input) {
            char_array_3[i++] = c;
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; i < 4; i++) {
                    result += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }

        if (i > 0) {
            for (j = i; j < 3; j++) {
                char_array_3[j] = 0;
            }

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; j < i + 1; j++) {
                result += base64_chars[char_array_4[j]];
            }

            while (i++ < 3) {
                result += '=';
            }
        }

        return result;
    }

    std::string base64Decode(const std::string& input) {
        std::string result;
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4];
        unsigned char char_array_3[3];

        for (char c : input) {
            if (c == '=') {
                break;
            }
            if (std::isspace(c)) {
                continue;
            }
            char_array_4[i++] = c;
            if (i == 4) {
                for (i = 0; i < 4; i++) {
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; i < 3; i++) {
                    result += char_array_3[i];
                }
                i = 0;
            }
        }

        if (i > 0) {
            for (j = i; j < 4; j++) {
                char_array_4[j] = 0;
            }

            for (j = 0; j < 4; j++) {
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; j < i - 1; j++) {
                result += char_array_3[j];
            }
        }

        return result;
    }

}
