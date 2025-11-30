#include "json.h"
#include <sstream>
#include <stdexcept>

namespace common {

// 序列化JSON值到字符串
std::string serializeJson(const JsonValue& value) {
    std::ostringstream oss;
    
    if (value.isNull()) {
        oss << "null";
    } else if (value.isBool()) {
        oss << (value.asBool() ? "true" : "false");
    } else if (value.isInt()) {
        oss << value.asInt();
    } else if (value.isDouble()) {
        oss << value.asDouble();
    } else if (value.isString()) {
        // 简单的字符串转义
        oss << '"';
        const std::string& str = value.asString();
        for (char c : str) {
            switch (c) {
                case '"': oss << "\\\"";
 break;
                case '\\': oss << "\\\\";
 break;
                case '\b': oss << "\\b";
 break;
                case '\f': oss << "\\f";
 break;
                case '\n': oss << "\\n";
 break;
                case '\r': oss << "\\r";
 break;
                case '\t': oss << "\\t";
 break;
                default:
                    oss << c;
            }
        }
        oss << '"';
    } else if (value.isArray()) {
        oss << '[';
        const auto& arr = value.asArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            oss << serializeJson(arr[i]);
            if (i < arr.size() - 1) {
                oss << ", ";
            }
        }
        oss << ']';
    } else if (value.isObject()) {
        oss << '{';
        const auto& obj = value.asObject();
        size_t count = 0;
        for (const auto& [key, val] : obj) {
            oss << '"' << key << '": ' << serializeJson(val);
            if (++count < obj.size()) {
                oss << ", ";
            }
        }
        oss << '}';
    }
    
    return oss.str();
}

// 简单的JSON解析器（仅支持基本功能）
JsonValue parseJson(const std::string& jsonStr) {
    // 这里实现一个简单的JSON解析器
    // 注意：这是一个简化版本，实际项目中应该使用更完善的解析器
    
    // 跳过空白字符的辅助函数
    auto skipWhitespace = [](const std::string& str, size_t& pos) {
        while (pos < str.size() && std::isspace(str[pos])) {
            ++pos;
        }
    };
    
    // 解析函数
    std::function<JsonValue(const std::string&, size_t&)> parseValue;
    std::function<JsonValue(const std::string&, size_t&)> parseObject;
    std::function<JsonValue(const std::string&, size_t&)> parseArray;
    std::function<JsonValue(const std::string&, size_t&)> parseString;
    
    parseValue = [&](const std::string& str, size_t& pos) -> JsonValue {
        skipWhitespace(str, pos);
        if (pos >= str.size()) {
            throw std::runtime_error("Unexpected end of JSON string");
        }
        
        char c = str[pos];
        if (c == '{') {
            return parseObject(str, pos);
        } else if (c == '[') {
            return parseArray(str, pos);
        } else if (c == '"') {
            return parseString(str, pos);
        } else if (c == 't') {
            if (str.substr(pos, 4) == "true") {
                pos += 4;
                return true;
            }
            throw std::runtime_error("Invalid JSON: expected 'true'");
        } else if (c == 'f') {
            if (str.substr(pos, 5) == "false") {
                pos += 5;
                return false;
            }
            throw std::runtime_error("Invalid JSON: expected 'false'");
        } else if (c == 'n') {
            if (str.substr(pos, 4) == "null") {
                pos += 4;
                return nullptr;
            }
            throw std::runtime_error("Invalid JSON: expected 'null'");
        } else if (isdigit(c) || c == '-') {
            // 解析数字
            size_t start = pos;
            bool hasDecimal = false;
            while (pos < str.size() && (isdigit(str[pos]) || str[pos] == '.' || str[pos] == '-' || str[pos] == 'e' || str[pos] == 'E')) {
                if (str[pos] == '.') {
                    hasDecimal = true;
                }
                ++pos;
            }
            std::string numStr = str.substr(start, pos - start);
            if (hasDecimal) {
                return std::stod(numStr);
            } else {
                return std::stoi(numStr);
            }
        }
        throw std::runtime_error("Invalid JSON value");
    };
    
    parseObject = [&](const std::string& str, size_t& pos) -> JsonValue {
        ++pos; // 跳过 '{'
        std::map<std::string, JsonValue> obj;
        
        while (true) {
            skipWhitespace(str, pos);
            if (pos >= str.size()) {
                throw std::runtime_error("Unexpected end of JSON object");
            }
            
            if (str[pos] == '}') {
                ++pos;
                return obj;
            }
            
            // 解析键
            std::string key = parseString(str, pos).asString();
            
            skipWhitespace(str, pos);
            if (pos >= str.size() || str[pos] != ':') {
                throw std::runtime_error("Expected ':' in JSON object");
            }
            ++pos;
            
            // 解析值
            JsonValue value = parseValue(str, pos);
            obj[key] = value;
            
            skipWhitespace(str, pos);
            if (pos >= str.size()) {
                throw std::runtime_error("Unexpected end of JSON object");
            }
            
            if (str[pos] == '}') {
                ++pos;
                return obj;
            } else if (str[pos] == ',') {
                ++pos;
            } else {
                throw std::runtime_error("Expected ',' or '}' in JSON object");
            }
        }
    };
    
    parseArray = [&](const std::string& str, size_t& pos) -> JsonValue {
        ++pos; // 跳过 '['
        std::vector<JsonValue> arr;
        
        while (true) {
            skipWhitespace(str, pos);
            if (pos >= str.size()) {
                throw std::runtime_error("Unexpected end of JSON array");
            }
            
            if (str[pos] == ']') {
                ++pos;
                return arr;
            }
            
            // 解析值
            JsonValue value = parseValue(str, pos);
            arr.push_back(value);
            
            skipWhitespace(str, pos);
            if (pos >= str.size()) {
                throw std::runtime_error("Unexpected end of JSON array");
            }
            
            if (str[pos] == ']') {
                ++pos;
                return arr;
            } else if (str[pos] == ',') {
                ++pos;
            } else {
                throw std::runtime_error("Expected ',' or ']' in JSON array");
            }
        }
    };
    
    parseString = [&](const std::string& str, size_t& pos) -> JsonValue {
        ++pos; // 跳过开始的 '"'
        std::string result;
        
        while (pos < str.size() && str[pos] != '"') {
            if (str[pos] == '\\') {
                ++pos;
                if (pos >= str.size()) {
                    throw std::runtime_error("Unexpected end of JSON string");
                }
                switch (str[pos]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += str[pos];
                }
            } else {
                result += str[pos];
            }
            ++pos;
        }
        
        if (pos >= str.size() || str[pos] != '"') {
            throw std::runtime_error("Unterminated JSON string");
        }
        ++pos; // 跳过结束的 '"'
        
        return result;
    };
    
    size_t pos = 0;
    JsonValue result = parseValue(jsonStr, pos);
    
    // 确保整个字符串都被解析
    skipWhitespace(jsonStr, pos);
    if (pos < jsonStr.size()) {
        throw std::runtime_error("Extra characters after JSON value");
    }
    
    return result;
}

} // namespace common
