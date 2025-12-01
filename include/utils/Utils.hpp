#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
#include <map>

namespace Utils {

    // 时间处理
    std::string getCurrentTimeStr();
    time_t getCurrentTime();
    std::string formatTime(time_t t, const std::string& format = "%Y-%m-%d %H:%M:%S");
    time_t parseTime(const std::string& time_str, const std::string& format = "%Y-%m-%d %H:%M:%S");
    bool isToday(time_t t);
    bool isThisMinute(time_t t);

    // 随机数生成
    std::string generateRandomString(size_t length);
    uint64_t generateRandomUint64();

    // JSON 解析与序列化（简化版）
    class JsonValue {
    public:
        enum Type { NULL_TYPE, BOOLEAN, NUMBER, STRING, ARRAY, OBJECT };

        JsonValue() : type_(NULL_TYPE) {};
        JsonValue(bool b) : type_(BOOLEAN), bool_value_(b) {};
        JsonValue(int64_t n) : type_(NUMBER), int_value_(n) {};
        JsonValue(double d) : type_(NUMBER), double_value_(d) {};
        JsonValue(const std::string& s) : type_(STRING), string_value_(s) {};
        JsonValue(const char* s) : type_(STRING), string_value_(s) {};
        JsonValue(Type t) : type_(t) {
            if (t == ARRAY) {
                array_value_ = std::vector<JsonValue>();
            } else if (t == OBJECT) {
                object_value_ = std::map<std::string, JsonValue>();
            }
        };

        Type getType() const { return type_; }

        bool asBool() const { return bool_value_; }
        int64_t asInt() const { return int_value_; }
        double asDouble() const { return double_value_; }
        std::string asString() const { return string_value_; }

        std::vector<JsonValue>& asArray() { return array_value_; }
        const std::vector<JsonValue>& asArray() const { return array_value_; }

        std::map<std::string, JsonValue>& asObject() { return object_value_; }
        const std::map<std::string, JsonValue>& asObject() const { return object_value_; }

        bool hasKey(const std::string& key) const;
        JsonValue& operator[](const std::string& key);
        const JsonValue& operator[](const std::string& key) const;

        std::string toString() const;

    private:
        Type type_;
        bool bool_value_;
        int64_t int_value_;
        double double_value_;
        std::string string_value_;
        std::vector<JsonValue> array_value_;
        std::map<std::string, JsonValue> object_value_;
    };

    JsonValue parseJson(const std::string& json_str);
    std::string jsonToString(const JsonValue& json);

    // 字符串处理
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string& str, const std::string& suffix);

    // Base64 编码
    std::string base64Encode(const std::string& input);
    std::string base64Decode(const std::string& input);
}

#endif // UTILS_HPP
