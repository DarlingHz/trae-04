#include <iostream>
#include <memory>
#include <crow.h>
#include "utils/Logger.h"
#include "utils/Config.h"
#include "utils/Database.h"
#include "repository/RiderRepository.h"
#include "repository/DriverRepository.h"
#include "repository/RideRequestRepository.h"
#include "repository/TripRepository.h"
#include "service/RiderService.h"
#include "service/DriverService.h"
#include "service/RideRequestService.h"
#include "service/TripService.h"
#include "service/MatchingService.h"

using namespace crow;
using namespace utils;
using namespace repository;
using namespace service;

int main() {
    try {
        // 初始化日志系统
        Logger::get_instance().init("ride-sharing-app.log");
        Logger::get_instance().set_level(utils::LogLevel::DEBUG);
        LOG_INFO << "日志系统初始化完成";

        // 初始化配置系统
        Config::get_instance().load("config/app.conf");
        LOG_INFO << "配置系统初始化完成"; 

        // 初始化数据库系统
        Database::get_instance().init("ride-sharing.db");
        LOG_INFO << "数据库系统初始化完成"; 

        // 创建数据访问层实例
        auto rider_repo = std::make_shared<RiderRepositoryImpl>();
        auto driver_repo = std::make_shared<DriverRepositoryImpl>();
        auto ride_request_repo = std::make_shared<RideRequestRepositoryImpl>();
        auto trip_repo = std::make_shared<TripRepositoryImpl>();

        // 创建服务层实例
        auto rider_service = std::make_shared<RiderService>(rider_repo);
        auto driver_service = std::make_shared<DriverService>(driver_repo);
        auto& matching_service = MatchingService::get_instance();
        auto ride_request_service = std::make_shared<RideRequestService>(ride_request_repo, matching_service);
        auto trip_service = std::make_shared<TripService>(trip_repo, driver_repo);

        // 初始化匹配服务
        matching_service.init(driver_repo, ride_request_repo, trip_repo);

        // 创建Crow应用实例
        App app;

        // 乘客相关路由
        CROW_ROUTE(app, "/riders").methods(HTTPMethod::POST)([rider_service](const request& req, response& res) {
            // 创建新乘客
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("无效的JSON数据");
                res.end();
                return;
            }

            model::Rider rider(0, json["name"].s(), json["phone"].s(), 5.0f, std::chrono::system_clock::now());
            int rider_id = rider_service->register_rider(rider);

            if (rider_id == -1) {
                res.code = 500;
                res.write("乘客注册失败");
                res.end();
                return;
            }

            res.code = 201;
            res.write(crow::json::wvalue({{"id", rider_id}, {"name", rider.get_name()}, {"phone", rider.get_phone()}}).dump());
            res.end();
        });

        CROW_ROUTE(app, "/riders/<int>").methods(HTTPMethod::GET)([rider_service](const request& req, response& res, int id) {
            // 获取乘客信息
            auto rider = rider_service->get_rider_by_id(id);
            if (!rider) {
                res.code = 404;
                res.write("乘客不存在");
                res.end();
                return;
            }

            res.code = 200;
            res.write(crow::json::wvalue({{"id", rider->get_id()}, {"name", rider->get_name()}, {"phone", rider->get_phone()}, {"rating", rider->get_rating()}}).dump());
            res.end();
        });

        // 车主相关路由
        CROW_ROUTE(app, "/drivers").methods(HTTPMethod::POST)([driver_service](const request& req, response& res) {
            // 创建新车主
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("无效的JSON数据");
                res.end();
                return;
            }

            model::Driver driver(0, json["name"].s(), json["license_plate"].s(), json["car_model"].s(), json["capacity"].i(), model::DriverStatus::AVAILABLE, 0, 0, 5.0f, std::chrono::system_clock::now());
            int driver_id = driver_service->register_driver(driver);

            if (driver_id == -1) {
                res.code = 500;
                res.write("车主注册失败");
                res.end();
                return;
            }

            res.code = 201;
            res.write(crow::json::wvalue({{"id", driver_id}, {"name", driver.get_name()}, {"license_plate", driver.get_license_plate()}, {"car_model", driver.get_car_model()}, {"capacity", driver.get_capacity()}}).dump());
            res.end();
        });

        CROW_ROUTE(app, "/drivers/<int>").methods(HTTPMethod::GET)([driver_service](const request& req, response& res, int id) {
            // 获取车主信息
            auto driver = driver_service->get_driver_by_id(id);
            if (!driver) {
                res.code = 404;
                res.write("车主不存在");
                res.end();
                return;
            }

            res.code = 200;
            res.write(crow::json::wvalue({{"id", driver->get_id()}, {"name", driver->get_name()}, {"license_plate", driver->get_license_plate()}, {"car_model", driver->get_car_model()}, {"capacity", driver->get_capacity()}, {"status", static_cast<int>(driver->get_status())}, {"current_x", driver->get_current_x()}, {"current_y", driver->get_current_y()}, {"rating", driver->get_rating()}}).dump());
            res.end();
        });

        // 行程请求相关路由
        CROW_ROUTE(app, "/ride-requests").methods(HTTPMethod::POST)([ride_request_service](const request& req, response& res) {
            // 创建新行程请求
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("无效的JSON数据");
                res.end();
                return;
            }

            auto now = std::chrono::system_clock::now();
            auto earliest_departure = now;
            auto latest_departure = now + std::chrono::hours(1);

            model::RideRequest ride_request(0, json["rider_id"].i(), json["start_x"].i(), json["start_y"].i(), json["end_x"].i(), json["end_y"].i(), earliest_departure, latest_departure, model::RideRequestStatus::PENDING, now);
            int ride_request_id = ride_request_service->create_ride_request(ride_request);

            if (ride_request_id == -1) {
                res.code = 500;
                res.write("行程请求创建失败");
                res.end();
                return;
            }

            res.code = 201;
            res.write(crow::json::wvalue({{"id", ride_request_id}, {"rider_id", ride_request.get_rider_id()}, {"start_x", ride_request.get_start_x()}, {"start_y", ride_request.get_start_y()}, {"end_x", ride_request.get_end_x()}, {"end_y", ride_request.get_end_y()}, {"status", static_cast<int>(ride_request.get_status())}}).dump());
            res.end();
        });

        // 行程相关路由
        CROW_ROUTE(app, "/trips/<int>/start").methods(HTTPMethod::POST)([trip_service](const request& req, response& res, int id) {
            // 开始行程
            if (trip_service->start_trip(id)) {
                res.code = 200;
                res.write("行程开始成功");
            } else {
                res.code = 500;
                res.write("行程开始失败");
            }
            res.end();
        });

        CROW_ROUTE(app, "/trips/<int>/end").methods(HTTPMethod::POST)([trip_service](const request& req, response& res, int id) {
            // 结束行程
            auto json = crow::json::load(req.body);
            if (!json) {
                res.code = 400;
                res.write("无效的JSON数据");
                res.end();
                return;
            }

            float fare = json["fare"].d();
            if (trip_service->end_trip(id, fare)) {
                res.code = 200;
                res.write("行程结束成功");
            } else {
                res.code = 500;
                res.write("行程结束失败");
            }
            res.end();
        });

        // 匹配服务相关路由
        CROW_ROUTE(app, "/matching/stats").methods(HTTPMethod::GET)([&matching_service](const request& req, response& res) {
            // 获取匹配统计信息
            std::string stats = matching_service.get_matching_stats();
            res.code = 200;
            res.write(stats);
            res.end();
        });

        // 启动服务器
        int port = Config::get_instance().get_int("server.port", 8080);
        LOG_INFO << "启动服务器，端口: " << port;
        app.port(port).multithreaded().run();

    } catch (const std::exception& e) {
        LOG_ERROR << "应用程序启动失败: " << e.what();
        std::cerr << "应用程序启动失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
