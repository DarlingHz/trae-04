#pragma once

#include <cpp-httplib/httplib.h>
#include <memory>
#include <nlohmann/json.hpp>
#include "../service/MovieService.h"

namespace controller {

class MovieController {
public:
    explicit MovieController(std::shared_ptr<service::MovieService> movie_service);
    
    // 处理 HTTP 请求的方法
    void createMovie(const httplib::Request& req, httplib::Response& res);
    void getMovieById(const httplib::Request& req, httplib::Response& res);
    void getMovies(const httplib::Request& req, httplib::Response& res);
    void updateMovie(const httplib::Request& req, httplib::Response& res);
    void deleteMovie(const httplib::Request& req, httplib::Response& res);
    
private:
    std::shared_ptr<service::MovieService> movie_service_;
    
    // 辅助方法
    nlohmann::json createResponse(int code, const std::string& message, const nlohmann::json& data = nlohmann::json::object());
};

} // namespace controller
