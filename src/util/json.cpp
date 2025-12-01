#include "json.h"
#include <iostream>

Result<json> JsonUtil::ParseJson(const std::string& json_str) {
  try {
    auto json_obj = json::parse(json_str);
    return Result<json>::Success(json_obj);
  } catch (const std::exception& e) {
    std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
    return Result<json>::Error("Invalid JSON format");
  }
}

std::string JsonUtil::ToJsonString(const json& json_obj, bool pretty) {
  try {
    if (pretty) {
      return json_obj.dump(2);
    } else {
      return json_obj.dump();
    }
  } catch (const std::exception& e) {
    std::cerr << "Failed to convert JSON to string: " << e.what() << std::endl;
    return "";
  }
}

std::string JsonUtil::GetString(const json& json_obj, const std::string& key, const std::string& default_value) {
  try {
    if (json_obj.contains(key) && json_obj[key].is_string()) {
      return json_obj[key].get<std::string>();
    } else {
      return default_value;
    }
  } catch (const std::exception& e) {
    std::cerr << "Failed to get string from JSON: " << e.what() << std::endl;
    return default_value;
  }
}

int JsonUtil::GetInt(const json& json_obj, const std::string& key, int default_value) {
  try {
    if (json_obj.contains(key) && json_obj[key].is_number_integer()) {
      return json_obj[key].get<int>();
    } else {
      return default_value;
    }
  } catch (const std::exception& e) {
    std::cerr << "Failed to get int from JSON: " << e.what() << std::endl;
    return default_value;
  }
}

bool JsonUtil::GetBool(const json& json_obj, const std::string& key, bool default_value) {
  try {
    if (json_obj.contains(key) && json_obj[key].is_boolean()) {
      return json_obj[key].get<bool>();
    } else {
      return default_value;
    }
  } catch (const std::exception& e) {
    std::cerr << "Failed to get bool from JSON: " << e.what() << std::endl;
    return default_value;
  }
}

std::vector<std::string> JsonUtil::GetStringArray(const json& json_obj, const std::string& key) {
  try {
    if (json_obj.contains(key) && json_obj[key].is_array()) {
      std::vector<std::string> array;
      for (const auto& item : json_obj[key]) {
        if (item.is_string()) {
          array.push_back(item.get<std::string>());
        }
      }
      return array;
    } else {
      return std::vector<std::string>();
    }
  } catch (const std::exception& e) {
    std::cerr << "Failed to get string array from JSON: " << e.what() << std::endl;
    return std::vector<std::string>();
  }
}
