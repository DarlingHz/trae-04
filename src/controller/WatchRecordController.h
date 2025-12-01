#pragma once

#include <cpp-httplib/httplib.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../service/WatchRecordService.h"

namespace controller {

class WatchRecordController {
public:
    explicit WatchRecordController(std::shared_ptr<service::WatchRecordService> watch_record_service);
    
    // 处理 HTTP 请求的方法
    void createWatchRecord(const httplib::Request& req, httplib::Response& res);
    void getWatchRecordsByUserId(const httplib::Request& req, httplib::Response& res);
    
private:
    std::shared_ptr<service::WatchRecordService> watch_record_service_;
    
    // 辅助方法
    nlohmann::json createResponse(int code, const std::string& message, const nlohmann::json& data = nlohmann::json::object());
};

} // namespace controller
