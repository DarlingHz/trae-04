#include "WatchRecordController.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <string>
#include <nlohmann/json.hpp>
#include <chrono>

namespace controller {

WatchRecordController::WatchRecordController(std::shared_ptr<service::WatchRecordService> watch_record_service)
    : watch_record_service_(std::move(watch_record_service)) {
}

void WatchRecordController::createWatchRecord(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体中的 JSON 数据
        auto json_body = nlohmann::json::parse(req.body);
        
        // 验证请求数据
        if (!json_body.contains("user_id") || !json_body["user_id"].is_number_integer() ||
            !json_body.contains("movie_id") || !json_body["movie_id"].is_number_integer() ||
            !json_body.contains("start_time") || !json_body["start_time"].is_string() ||
            !json_body.contains("watch_duration") || !json_body["watch_duration"].is_number_integer() ||
            !json_body.contains("is_finished") || !json_body["is_finished"].is_boolean()) {
            LOG_ERROR("Invalid request: user_id, movie_id, start_time, watch_duration, and is_finished are required");
            auto response = createResponse(400, "Invalid request: user_id, movie_id, start_time, watch_duration, and is_finished are required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        int user_id = json_body["user_id"];
        int movie_id = json_body["movie_id"];
        std::string start_time_str = json_body["start_time"];
        int watch_duration = json_body["watch_duration"];
        bool is_finished = json_body["is_finished"];
        
        // 解析开始时间字符串
        auto start_time = utils::TimeUtils::isoStringToTimePoint(start_time_str);
        
        // 提取可选的评分和备注
        double rating = json_body.contains("rating") && json_body["rating"].is_number() ? json_body["rating"].get<double>() : 0.0;
        std::string comment = json_body.contains("note") && json_body["note"].is_string() ? json_body["note"].get<std::string>() : "";
        
        // 调用服务层创建观影记录
        auto record = watch_record_service_->createWatchRecord(user_id, movie_id, start_time, watch_duration, is_finished, rating, comment);
        
        if (record) {
            LOG_INFO("Watch record created successfully: " + std::to_string(record->getId()));
            
            // 构造响应数据
            nlohmann::json record_json;
            record_json["id"] = record->getId();
            record_json["user_id"] = record->getUserId();
            record_json["movie_id"] = record->getMovieId();
            record_json["start_time"] = utils::TimeUtils::timePointToIsoString(record->getStartTime());
            record_json["watch_duration"] = record->getWatchDuration();
            record_json["is_finished"] = record->getIsFinished();
            record_json["rating"] = record->getRating();
            if (!record->getComment().empty()) {
                record_json["comment"] = record->getComment();
            }
            record_json["created_at"] = utils::TimeUtils::timePointToIsoString(record->getCreatedAt());
            
            auto response = createResponse(0, "ok", record_json);
            res.set_content(response.dump(), "application/json");
            res.status = 201;
        } else {
            LOG_ERROR("Failed to create watch record");
            auto response = createResponse(500, "Failed to create watch record");
            res.set_content(response.dump(), "application/json");
            res.status = 500;
        }
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("JSON parse error: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid JSON format");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error: " + std::string(e.what()));
        auto response = createResponse(500, "Internal server error");
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

void WatchRecordController::getWatchRecordsByUserId(const httplib::Request& req, httplib::Response& res) {
    try {
        // 从 URL 参数中获取用户 ID
        auto it = req.params.find("id");
        if (it == req.params.end()) {
            LOG_ERROR("Invalid request: user ID is required");
            auto response = createResponse(400, "Invalid request: user ID is required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        int user_id = std::stoi(it->second);
        
        // 从查询参数中获取时间范围和分页信息
        std::string start_time_str = req.get_param_value("start_time");
        std::string end_time_str = req.get_param_value("end_time");
        std::string page_str = req.get_param_value("page");
        std::string page_size_str = req.get_param_value("page_size");
        
        // 解析时间范围
        std::optional<std::chrono::system_clock::time_point> start_time;
        std::optional<std::chrono::system_clock::time_point> end_time;
        
        if (!start_time_str.empty()) {
            start_time = utils::TimeUtils::isoStringToTimePoint(start_time_str);
            if (!start_time) {
                LOG_ERROR("Invalid start_time format: " + start_time_str);
                auto response = createResponse(400, "Invalid start_time format: must be ISO 8601");
                res.set_content(response.dump(), "application/json");
                res.status = 400;
                return;
            }
        }
        
        if (!end_time_str.empty()) {
            end_time = utils::TimeUtils::isoStringToTimePoint(end_time_str);
            if (!end_time) {
                LOG_ERROR("Invalid end_time format: " + end_time_str);
                auto response = createResponse(400, "Invalid end_time format: must be ISO 8601");
                res.set_content(response.dump(), "application/json");
                res.status = 400;
                return;
            }
        }
        
        // 解析分页信息，设置默认值
        int page = (page_str.empty()) ? 1 : std::stoi(page_str);
        int page_size = (page_size_str.empty()) ? 10 : std::stoi(page_size_str);
        
        // 调用服务层获取用户的观影记录
        auto records = watch_record_service_->getWatchRecordsByUserId(user_id, start_time, end_time, page, page_size);
        
        LOG_INFO("Retrieved " + std::to_string(records.size()) + " watch records for user: " + std::to_string(user_id));
        
        // 构造响应数据
        nlohmann::json records_json = nlohmann::json::array();
        for (const auto& record : records) {
            nlohmann::json record_json;
            record_json["id"] = record->getId();
            record_json["user_id"] = record->getUserId();
            record_json["movie_id"] = record->getMovieId();
            record_json["start_time"] = utils::TimeUtils::timePointToIsoString(record->getStartTime());
            record_json["watch_duration"] = record->getWatchDuration();
            record_json["is_finished"] = record->getIsFinished();
            record_json["rating"] = record->getRating();
            if (!record->getComment().empty()) {
                record_json["comment"] = record->getComment();
            }
            record_json["created_at"] = utils::TimeUtils::timePointToIsoString(record->getCreatedAt());
            records_json.push_back(record_json);
        }
        
        auto response = createResponse(0, "ok", records_json);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("Invalid parameter: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid parameter");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error: " + std::string(e.what()));
        auto response = createResponse(500, "Internal server error");
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

nlohmann::json WatchRecordController::createResponse(int code, const std::string& message, const nlohmann::json& data) {
    nlohmann::json response;
    response["code"] = code;
    response["message"] = message;
    response["data"] = data;
    return response;
}

} // namespace controller
