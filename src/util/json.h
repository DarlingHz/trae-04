#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "result.h"

using json = nlohmann::json;

class JsonUtil {
public:
  // 将JSON字符串解析为json对象
  static Result<json> ParseJson(const std::string& json_str);
  
  // 将json对象转换为JSON字符串
  static std::string ToJsonString(const json& json_obj, bool pretty = false);
  
  // 从json对象中获取字符串值
  static std::string GetString(const json& json_obj, const std::string& key, const std::string& default_value = "");
  
  // 从json对象中获取整数值
  static int GetInt(const json& json_obj, const std::string& key, int default_value = 0);
  
  // 从json对象中获取布尔值
  static bool GetBool(const json& json_obj, const std::string& key, bool default_value = false);
  
  // 从json对象中获取字符串数组
  static std::vector<std::string> GetStringArray(const json& json_obj, const std::string& key);
};
