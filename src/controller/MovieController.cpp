#include "MovieController.h"
#include "../utils/Logger.h"
#include "../utils/TimeUtils.h"
#include <string>
#include <nlohmann/json.hpp>

namespace controller {

MovieController::MovieController(std::shared_ptr<service::MovieService> movie_service)
    : movie_service_(std::move(movie_service)) {
}

void MovieController::createMovie(const httplib::Request& req, httplib::Response& res) {
    try {
        // 解析请求体中的 JSON 数据
        auto json_body = nlohmann::json::parse(req.body);
        
        // 验证请求数据
        if (!json_body.contains("title") || !json_body["title"].is_string() ||
            !json_body.contains("type") || !json_body["type"].is_string() ||
            !json_body.contains("duration") || !json_body["duration"].is_number_integer()) {
            LOG_ERROR("Invalid request: title, type, and duration are required");
            auto response = createResponse(400, "Invalid request: title, type, and duration are required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        std::string title = json_body["title"];
        std::string type = json_body["type"];
        int duration = json_body["duration"];
        
        // 调用服务层创建影片
        auto movie = movie_service_->createMovie(title, type, duration);
        
        if (movie) {
            LOG_INFO("Movie created successfully: " + std::to_string(movie->getId()));
            
            // 构造响应数据
            nlohmann::json movie_json;
            movie_json["id"] = movie->getId();
            movie_json["title"] = movie->getTitle();
            movie_json["type"] = movie->getType();
            movie_json["duration"] = movie->getDuration();
            movie_json["status"] = movie->getStatus();
            movie_json["created_at"] = utils::TimeUtils::timePointToIsoString(movie->getCreatedAt());
            
            auto response = createResponse(0, "ok", movie_json);
            res.set_content(response.dump(), "application/json");
            res.status = 201;
        } else {
            LOG_ERROR("Failed to create movie");
            auto response = createResponse(500, "Failed to create movie");
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

void MovieController::getMovieById(const httplib::Request& req, httplib::Response& res) {
    try {
        // 从 URL 参数中获取影片 ID
        auto it = req.params.find("id");
        if (it == req.params.end()) {
            LOG_ERROR("Invalid request: movie ID is required");
            auto response = createResponse(400, "Invalid request: movie ID is required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        int movie_id = std::stoi(it->second);
        
        // 调用服务层获取影片信息
        auto movie = movie_service_->getMovieById(movie_id);
        
        if (movie) {
            LOG_INFO("Movie retrieved successfully: " + std::to_string(movie->getId()));
            
            // 构造响应数据
            nlohmann::json movie_json;
            movie_json["id"] = movie->getId();
            movie_json["title"] = movie->getTitle();
            movie_json["type"] = movie->getType();
            movie_json["duration"] = movie->getDuration();
            movie_json["status"] = movie->getStatus();
            movie_json["created_at"] = utils::TimeUtils::timePointToIsoString(movie->getCreatedAt());
            
            auto response = createResponse(0, "ok", movie_json);
            res.set_content(response.dump(), "application/json");
            res.status = 200;
        } else {
            LOG_ERROR("Movie not found: " + std::to_string(movie_id));
            auto response = createResponse(404, "Movie not found");
            res.set_content(response.dump(), "application/json");
            res.status = 404;
        }
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("Invalid movie ID: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid movie ID");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error: " + std::string(e.what()));
        auto response = createResponse(500, "Internal server error");
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

void MovieController::getMovies(const httplib::Request& req, httplib::Response& res) {
    try {
        // 从查询参数中获取搜索条件和分页信息
        std::string keyword = req.get_param_value("keyword");
        std::string type = req.get_param_value("type");
        std::string page_str = req.get_param_value("page");
        std::string page_size_str = req.get_param_value("page_size");
        
        // 解析分页信息，设置默认值
        int page = (page_str.empty()) ? 1 : std::stoi(page_str);
        int page_size = (page_size_str.empty()) ? 10 : std::stoi(page_size_str);
        
        // 调用服务层获取影片列表
        auto movies = movie_service_->getMovies(keyword, type, page, page_size);
        
        LOG_INFO("Retrieved " + std::to_string(movies.size()) + " movies");
        
        // 构造响应数据
        nlohmann::json movies_json = nlohmann::json::array();
        for (const auto& movie : movies) {
            nlohmann::json movie_json;
            movie_json["id"] = movie->getId();
            movie_json["title"] = movie->getTitle();
            movie_json["type"] = movie->getType();
            movie_json["duration"] = movie->getDuration();
            movie_json["status"] = movie->getStatus();
            movie_json["created_at"] = utils::TimeUtils::timePointToIsoString(movie->getCreatedAt());
            movies_json.push_back(movie_json);
        }
        
        auto response = createResponse(0, "ok", movies_json);
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

void MovieController::updateMovie(const httplib::Request& req, httplib::Response& res) {
    try {
        // 从 URL 参数中获取影片 ID
        auto it = req.params.find("id");
        if (it == req.params.end()) {
            LOG_ERROR("Invalid request: movie ID is required");
            auto response = createResponse(400, "Invalid request: movie ID is required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        int movie_id = std::stoi(it->second);
        
        // 解析请求体中的 JSON 数据
        auto json_body = nlohmann::json::parse(req.body);
        
        // 验证请求数据
        if ((json_body.contains("title") && !json_body["title"].is_string()) ||
            (json_body.contains("type") && !json_body["type"].is_string()) ||
            (json_body.contains("duration") && !json_body["duration"].is_number_integer())) {
            LOG_ERROR("Invalid request: title, type, and duration must be valid types");
            auto response = createResponse(400, "Invalid request: title, type, and duration must be valid types");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        // 提取更新数据
        std::string title = json_body.contains("title") ? json_body["title"].get<std::string>() : "";
        std::string type = json_body.contains("type") ? json_body["type"].get<std::string>() : "";
        int duration = json_body.contains("duration") ? json_body["duration"].get<int>() : 0;
        
        // 调用服务层更新影片
        bool success = movie_service_->updateMovie(movie_id, title, type, duration);
        
        if (success) {
            // 获取更新后的影片信息
            auto movie = movie_service_->getMovieById(movie_id);
            
            if (movie) {
                LOG_INFO("Movie updated successfully: " + std::to_string(movie->getId()));
                
                // 构造响应数据
                nlohmann::json movie_json;
                movie_json["id"] = movie->getId();
                movie_json["title"] = movie->getTitle();
                movie_json["type"] = movie->getType();
                movie_json["duration"] = movie->getDuration();
                movie_json["status"] = movie->getStatus();
                movie_json["created_at"] = utils::TimeUtils::timePointToIsoString(movie->getCreatedAt());
                
                auto response = createResponse(0, "ok", movie_json);
                res.set_content(response.dump(), "application/json");
                res.status = 200;
            } else {
                LOG_ERROR("Failed to retrieve updated movie: " + std::to_string(movie_id));
                auto response = createResponse(500, "Failed to retrieve updated movie");
                res.set_content(response.dump(), "application/json");
                res.status = 500;
            }
        } else {
            LOG_ERROR("Failed to update movie: " + std::to_string(movie_id));
            auto response = createResponse(404, "Movie not found or failed to update");
            res.set_content(response.dump(), "application/json");
            res.status = 404;
        }
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("JSON parse error: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid JSON format");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
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

void MovieController::deleteMovie(const httplib::Request& req, httplib::Response& res) {
    try {
        // 从 URL 参数中获取影片 ID
        auto it = req.params.find("id");
        if (it == req.params.end()) {
            LOG_ERROR("Invalid request: movie ID is required");
            auto response = createResponse(400, "Invalid request: movie ID is required");
            res.set_content(response.dump(), "application/json");
            res.status = 400;
            return;
        }
        
        int movie_id = std::stoi(it->second);
        
        // 调用服务层删除影片
        bool success = movie_service_->deleteMovie(movie_id);
        
        if (success) {
            LOG_INFO("Movie deleted successfully: " + std::to_string(movie_id));
            auto response = createResponse(0, "ok");
            res.set_content(response.dump(), "application/json");
            res.status = 200;
        } else {
            LOG_ERROR("Failed to delete movie: " + std::to_string(movie_id));
            auto response = createResponse(404, "Movie not found or failed to delete");
            res.set_content(response.dump(), "application/json");
            res.status = 404;
        }
    } catch (const std::invalid_argument& e) {
        LOG_ERROR("Invalid movie ID: " + std::string(e.what()));
        auto response = createResponse(400, "Invalid movie ID");
        res.set_content(response.dump(), "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected error: " + std::string(e.what()));
        auto response = createResponse(500, "Internal server error");
        res.set_content(response.dump(), "application/json");
        res.status = 500;
    }
}

nlohmann::json MovieController::createResponse(int code, const std::string& message, const nlohmann::json& data) {
    nlohmann::json response;
    response["code"] = code;
    response["message"] = message;
    response["data"] = data;
    return response;
}

} // namespace controller
