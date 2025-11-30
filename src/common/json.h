#pragma once
#include <string>
#include <map>
#include <vector>
#include <variant>

namespace common {

// 定义JsonValue类型，使用std::variant支持多种数据类型
class JsonValue {
public:
    using ValueType = std::variant<
        std::nullptr_t,
        bool,
        int,
        double,
        std::string,
        std::vector<JsonValue>,
        std::map<std::string, JsonValue>
    >;

    // 构造函数
    JsonValue() : value_(nullptr) {}
    JsonValue(std::nullptr_t) : value_(nullptr) {}
    JsonValue(bool b) : value_(b) {}
    JsonValue(int i) : value_(i) {}
    JsonValue(double d) : value_(d) {}
    JsonValue(const std::string& s) : value_(s) {}
    JsonValue(const char* s) : value_(std::string(s)) {}
    JsonValue(const std::vector<JsonValue>& arr) : value_(arr) {}
    JsonValue(const std::map<std::string, JsonValue>& obj) : value_(obj) {}

    // 访问器方法
    bool isNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
    bool isBool() const { return std::holds_alternative<bool>(value_); }
    bool isInt() const { return std::holds_alternative<int>(value_); }
    bool isDouble() const { return std::holds_alternative<double>(value_); }
    bool isString() const { return std::holds_alternative<std::string>(value_); }
    bool isArray() const { return std::holds_alternative<std::vector<JsonValue>>(value_); }
    bool isObject() const { return std::holds_alternative<std::map<std::string, JsonValue>>(value_); }

    // 获取值的方法
    bool asBool() const { return std::get<bool>(value_); }
    int asInt() const { return std::get<int>(value_); }
    double asDouble() const { return std::get<double>(value_); }
    const std::string& asString() const { return std::get<std::string>(value_); }
    const std::vector<JsonValue>& asArray() const { return std::get<std::vector<JsonValue>>(value_); }
    const std::map<std::string, JsonValue>& asObject() const { return std::get<std::map<std::string, JsonValue>>(value_); }

    // 操作符重载
    JsonValue& operator[](const std::string& key) {
        if (!isObject()) {
            value_ = std::map<std::string, JsonValue>();
        }
        return std::get<std::map<std::string, JsonValue>>(value_)[key];
    }

    JsonValue& operator[](size_t index) {
        if (!isArray()) {
            value_ = std::vector<JsonValue>();
        }
        auto& arr = std::get<std::vector<JsonValue>>(value_);
        if (index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }

private:
    ValueType value_;
};

// JSON工具函数
std::string serializeJson(const JsonValue& value);
JsonValue parseJson(const std::string& jsonStr);

} // namespace common
