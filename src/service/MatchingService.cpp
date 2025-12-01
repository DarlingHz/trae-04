#include "service/MatchingService.h"
#include "utils/Logger.h"
#include <algorithm>
#include <chrono>

namespace service {

MatchingService::MatchingService() 
    : total_matching_attempts_(0)
    , successful_matches_(0)
    , total_pending_requests_(0)
    , total_available_drivers_(0) {
}

MatchingService::~MatchingService() {
}

MatchingService& MatchingService::get_instance() {
    static MatchingService instance;
    return instance;
}

void MatchingService::init(std::shared_ptr<repository::DriverRepository> driver_repo,
                           std::shared_ptr<repository::RideRequestRepository> ride_request_repo,
                           std::shared_ptr<repository::TripRepository> trip_repo) {
    driver_repo_ = driver_repo;
    ride_request_repo_ = ride_request_repo;
    trip_repo_ = trip_repo;

    LOG_INFO << "匹配服务初始化完成"; 
}

void MatchingService::trigger_matching() {
    std::lock_guard<std::mutex> lock(matching_mutex_);

    total_matching_attempts_++;

    // 获取待匹配的行程请求
    auto pending_requests = ride_request_repo_->get_pending();
    total_pending_requests_ = pending_requests.size();

    // 获取可用的车主
    auto available_drivers = driver_repo_->get_available();
    total_available_drivers_ = available_drivers.size();

    LOG_DEBUG << "触发匹配: 待匹配请求数=" << pending_requests.size() << ", 可用车主数=" << available_drivers.size(); 

    if (pending_requests.empty() || available_drivers.empty()) {
        LOG_DEBUG << "没有待匹配的请求或可用的车主，跳过匹配"; 
        return;
    }

    // 执行匹配算法
    perform_matching(pending_requests, available_drivers);
}

void MatchingService::on_new_ride_request(const model::RideRequest& ride_request) {
    LOG_INFO << "收到新的行程请求: ID=" << ride_request.get_id(); 
    trigger_matching();
}

void MatchingService::on_driver_status_update(const model::Driver& driver) {
    LOG_INFO << "车主状态更新: ID=" << driver.get_id() << ", 状态=" << static_cast<int>(driver.get_status()); 
    
    // 如果车主状态变为可用，则触发匹配
    if (driver.get_status() == model::DriverStatus::AVAILABLE) {
        trigger_matching();
    }
}

void MatchingService::on_driver_location_update(const model::Driver& driver) {
    LOG_DEBUG << "车主位置更新: ID=" << driver.get_id() << ", 位置=({" << driver.get_current_x() << ", " << driver.get_current_y() << "})"; 
    
    // 车主位置更新时触发匹配
    trigger_matching();
}

std::string MatchingService::get_matching_stats() {
    std::lock_guard<std::mutex> lock(matching_mutex_);

    float success_rate = 0.0f;
    if (total_matching_attempts_ > 0) {
        success_rate = static_cast<float>(successful_matches_) / total_matching_attempts_ * 100.0f;
    }

    return "匹配统计信息:\n" 
           "  总匹配尝试次数: " + std::to_string(total_matching_attempts_) + "\n" 
           "  成功匹配次数: " + std::to_string(successful_matches_) + "\n" 
           "  匹配成功率: " + std::to_string(success_rate) + "%\n" 
           "  当前待匹配请求数: " + std::to_string(total_pending_requests_) + "\n" 
           "  当前可用车主数: " + std::to_string(total_available_drivers_) + "\n";
}

void MatchingService::perform_matching(const std::vector<model::RideRequest>& pending_requests,
                                         const std::vector<model::Driver>& available_drivers) {
    LOG_DEBUG << "开始执行匹配算法"; 

    // 为每个待匹配的请求尝试找到最佳匹配的车主
    for (const auto& request : pending_requests) {
        auto best_match = select_best_match(request, available_drivers);

        if (best_match) {
            LOG_INFO << "匹配成功: 行程请求ID=" << request.get_id() << ", 车主ID=" << best_match->get_id(); 

            // 完成匹配操作
            if (complete_matching(request, *best_match)) {
                successful_matches_++;
            }
        } else {
            LOG_DEBUG << "未找到匹配的车主: 行程请求ID=" << request.get_id(); 
        }
    }

    LOG_DEBUG << "匹配算法执行完成"; 
}

int MatchingService::calculate_manhattan_distance(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

int MatchingService::calculate_euclidean_distance_squared(int x1, int y1, int x2, int y2) {
    int dx = x1 - x2;
    int dy = y1 - y2;
    return dx * dx + dy * dy;
}

std::optional<model::Driver> MatchingService::select_best_match(const model::RideRequest& request,
                                                                      const std::vector<model::Driver>& available_drivers) {
    std::optional<model::Driver> best_match;
    int best_distance = MATCHING_RADIUS_THRESHOLD + 1; // 初始化为超过阈值的距离

    for (const auto& driver : available_drivers) {
        // 计算车主当前位置与乘客起点的距离
        int distance = calculate_manhattan_distance(
            driver.get_current_x(), driver.get_current_y(),
            request.get_start_x(), request.get_start_y()
        );

        // 如果距离超过阈值，则跳过该车主
        if (distance > MATCHING_RADIUS_THRESHOLD) {
            continue;
        }

        // 选择距离最近的车主
        if (distance < best_distance) {
            best_distance = distance;
            best_match = driver;
        } else if (distance == best_distance) {
            // 如果距离相同，则选择注册时间最早的车主（作为次要排序条件）
            if (best_match) {
                if (driver.get_registration_time() < best_match->get_registration_time()) {
                    best_match = driver;
                }
            } else {
                best_match = driver;
            }
        }
    }

    return best_match;
}

bool MatchingService::complete_matching(const model::RideRequest& request, const model::Driver& driver) {
    try {
        // 1. 更新行程请求状态为已匹配
        model::RideRequest updated_request = request;
        updated_request.set_status(model::RideRequestStatus::MATCHED);
        if (!ride_request_repo_->update(updated_request)) {
            LOG_ERROR << "更新行程请求状态失败: ID=" << request.get_id(); 
            return false;
        }

        // 2. 更新车主状态为正在行程中
        model::Driver updated_driver = driver;
        updated_driver.set_status(model::DriverStatus::ON_TRIP);
        if (!driver_repo_->update_status(driver.get_id(), model::DriverStatus::ON_TRIP)) {
            LOG_ERROR << "更新车主状态失败: ID=" << driver.get_id(); 
            
            // 回滚行程请求状态
            model::RideRequest rollback_request = request;
            rollback_request.set_status(model::RideRequestStatus::PENDING);
            ride_request_repo_->update(rollback_request);
            
            return false;
        }

        // 3. 创建新的行程记录
        auto now = std::chrono::system_clock::now();
        model::Trip trip(0, driver.get_id(), request.get_rider_id(), request.get_id(), 
                          now, now, now, model::TripStatus::ONGOING, 0.0f);

        int trip_id = trip_repo_->create(trip);
        if (trip_id == -1) {
            LOG_ERROR << "创建行程记录失败"; 
            
            // 回滚行程请求状态和车主状态
            model::RideRequest rollback_request = request;
            rollback_request.set_status(model::RideRequestStatus::PENDING);
            ride_request_repo_->update(rollback_request);
            
            driver_repo_->update_status(driver.get_id(), model::DriverStatus::AVAILABLE);
            
            return false;
        }

        LOG_INFO << "行程创建成功: ID=" << trip_id; 
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR << "完成匹配时发生异常: " << e.what(); 
        return false;
    }
}

} // namespace service
