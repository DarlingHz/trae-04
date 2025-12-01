#include "time.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

int64_t TimeUtil::GetCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::system_clock::to_time_t(now);
}

std::string TimeUtil::TimestampToString(int64_t timestamp) {
  auto time = std::chrono::system_clock::from_time_t(timestamp);
  time_t t = std::chrono::system_clock::to_time_t(time);
  auto tm = std::localtime(&t);
  
  std::ostringstream oss;
  oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
  
  return oss.str();
}

int64_t TimeUtil::StringToTimestamp(const std::string& time_str) {
  std::tm tm = {};
  std::istringstream iss(time_str);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  
  if (iss.fail()) {
    return -1;
  }
  
  return std::mktime(&tm);
}
