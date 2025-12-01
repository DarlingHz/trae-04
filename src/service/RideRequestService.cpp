#include "service/RideRequestService.h"
#include "utils/Logger.h"
#include <chrono>

namespace service {

RideRequestService::RideRequestService(std::shared_ptr<repository::RideRequestRepository> ride_request_repo,
                                         MatchingService& matching_service)
    : ride_request_repo_(ride_request_repo)
    , matching_service_(matching_service) {
}

RideRequestService::~RideRequestService() {
}

int RideRequestService::create_ride_request(const model::RideRequest& ride_request) {
    LOG_INFO << "开始创建行程请求: 乘客ID=" << ride_request.get_rider_id() << ", 起点=({" << ride_request.get_start_x() << ", " << ride_request.get_start_y() << "}), 终点=({" << ride_request.get_end_x() << ", " << ride_request.get_end_y() << ")";

    // 验证行程请求信息
    if (!validate_ride_request(ride_request)) {
        LOG_ERROR << "行程请求信息验证失败";
        return -1;
    }

    // 创建行程请求
    int ride_request_id = ride_request_repo_->create(ride_request);
    if (ride_request_id == -1) {
        LOG_ERROR << "行程请求创建失败";
        return -1;
    }

    LOG_INFO << "行程请求创建成功: ID=" << ride_request_id;

    // 获取创建后的行程请求信息
    auto created_request = ride_request_repo_->get_by_id(ride_request_id);
    if (created_request) {
        // 通知匹配服务有新的行程请求
        matching_service_.on_new_ride_request(*created_request);
    } else {
        LOG_WARNING << "无法获取刚创建的行程请求信息: ID=" << ride_request_id;
    }

    return ride_request_id;
}

std::optional<model::RideRequest> RideRequestService::get_ride_request_by_id(int id) {
    LOG_DEBUG << "获取行程请求信息: ID=" << id;

    auto ride_request = ride_request_repo_->get_by_id(id);
    if (ride_request) {
        LOG_DEBUG << "行程请求信息获取成功: ID=" << id << ", 乘客ID=" << ride_request->get_rider_id() << ", 状态=" << static_cast<int>(ride_request->get_status());
    } else {
        LOG_DEBUG << "未找到行程请求: ID=" << id;
    }

    return ride_request;
}

std::vector<model::RideRequest> RideRequestService::get_all_ride_requests() {
    LOG_DEBUG << "获取所有行程请求信息";

    auto ride_requests = ride_request_repo_->get_all();
    LOG_DEBUG << "获取行程请求信息成功: 总数=" << ride_requests.size();

    return ride_requests;
}

std::vector<model::RideRequest> RideRequestService::get_pending_ride_requests() {
    LOG_DEBUG << "获取所有待匹配行程请求信息";

    auto ride_requests = ride_request_repo_->get_pending();
    LOG_DEBUG << "获取待匹配行程请求信息成功: 总数=" << ride_requests.size();

    return ride_requests;
}

std::vector<model::RideRequest> RideRequestService::get_ride_requests_by_rider_id(int rider_id) {
    LOG_DEBUG << "根据乘客ID获取行程请求信息: 乘客ID=" << rider_id;

    auto ride_requests = ride_request_repo_->get_by_rider_id(rider_id);
    LOG_DEBUG << "根据乘客ID获取行程请求信息成功: 总数=" << ride_requests.size();

    return ride_requests;
}

bool RideRequestService::update_ride_request(const model::RideRequest& ride_request) {
    LOG_INFO << "更新行程请求信息: ID=" << ride_request.get_id();

    // 验证行程请求信息
    if (!validate_ride_request(ride_request)) {
        LOG_ERROR << "行程请求信息验证失败";
        return false;
    }

    // 检查行程请求是否存在
    auto existing_request = ride_request_repo_->get_by_id(ride_request.get_id());
    if (!existing_request) {
        LOG_ERROR << "行程请求不存在: ID=" << ride_request.get_id();
        return false;
    }

    // 更新行程请求信息
    if (!ride_request_repo_->update(ride_request)) {
        LOG_ERROR << "行程请求信息更新失败: ID=" << ride_request.get_id();
        return false;
    }

    LOG_INFO << "行程请求信息更新成功: ID=" << ride_request.get_id();
    return true;
}

bool RideRequestService::update_ride_request_status(int id, model::RideRequestStatus status) {
    LOG_INFO << "更新行程请求状态: ID=" << id << ", 新状态=" << static_cast<int>(status);

    // 检查行程请求是否存在
    auto existing_request = ride_request_repo_->get_by_id(id);
    if (!existing_request) {
        LOG_ERROR << "行程请求不存在: ID=" << id;
        return false;
    }

    // 更新行程请求状态
    if (!ride_request_repo_->update_status(id, status)) {
        LOG_ERROR << "行程请求状态更新失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程请求状态更新成功: ID=" << id;
    return true;
}

bool RideRequestService::cancel_ride_request(int id) {
    LOG_INFO << "取消行程请求: ID=" << id;

    // 检查行程请求是否存在
    auto existing_request = ride_request_repo_->get_by_id(id);
    if (!existing_request) {
        LOG_ERROR << "行程请求不存在: ID=" << id;
        return false;
    }

    // 检查行程请求是否可以取消（只有待匹配或已匹配但未开始的行程可以取消）
    if (existing_request->get_status() == model::RideRequestStatus::COMPLETED || 
        existing_request->get_status() == model::RideRequestStatus::CANCELLED) {
        LOG_ERROR << "行程请求无法取消: ID=" << id << ", 当前状态=" << static_cast<int>(existing_request->get_status());
        return false;
    }

    // 更新行程请求状态为已取消
    if (!ride_request_repo_->update_status(id, model::RideRequestStatus::CANCELLED)) {
        LOG_ERROR << "行程请求取消失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程请求取消成功: ID=" << id;
    return true;
}

bool RideRequestService::delete_ride_request(int id) {
    LOG_INFO << "删除行程请求: ID=" << id;

    // 检查行程请求是否存在
    auto existing_request = ride_request_repo_->get_by_id(id);
    if (!existing_request) {
        LOG_ERROR << "行程请求不存在: ID=" << id;
        return false;
    }

    // 删除行程请求
    if (!ride_request_repo_->remove(id)) {
        LOG_ERROR << "行程请求删除失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程请求删除成功: ID=" << id;
    return true;
}

bool RideRequestService::validate_ride_request(const model::RideRequest& ride_request) {
    // 验证乘客ID是否合法
    if (ride_request.get_rider_id() <= 0) {
        LOG_ERROR << "乘客ID不合法: 乘客ID=" << ride_request.get_rider_id();
        return false;
    }

    // 验证起点坐标是否合法（假设坐标范围为0-1000）
    if (ride_request.get_start_x() < 0 || ride_request.get_start_x() > 1000 || 
        ride_request.get_start_y() < 0 || ride_request.get_start_y() > 1000) {
        LOG_ERROR << "起点坐标不合法: 起点=({" << ride_request.get_start_x() << ", " << ride_request.get_start_y() << ")";
        return false;
    }

    // 验证终点坐标是否合法（假设坐标范围为0-1000）
    if (ride_request.get_end_x() < 0 || ride_request.get_end_x() > 1000 || 
        ride_request.get_end_y() < 0 || ride_request.get_end_y() > 1000) {
        LOG_ERROR << "终点坐标不合法: 终点=({" << ride_request.get_end_x() << ", " << ride_request.get_end_y() << ")";
        return false;
    }

    // 验证最早出发时间是否早于最晚出发时间
    if (ride_request.get_earliest_departure() >= ride_request.get_latest_departure()) {
        LOG_ERROR << "出发时间窗口不合法: 最早出发时间=" << std::chrono::system_clock::to_time_t(ride_request.get_earliest_departure()) << ", 最晚出发时间=" << std::chrono::system_clock::to_time_t(ride_request.get_latest_departure());
        return false;
    }

    // 验证出发时间是否在合理范围内（不早于当前时间，不晚于当前时间+7天）
    auto now = std::chrono::system_clock::now();
    auto seven_days_later = now + std::chrono::hours(24 * 7);

    if (ride_request.get_earliest_departure() < now || ride_request.get_latest_departure() > seven_days_later) {
        LOG_ERROR << "出发时间不在合理范围内: 最早出发时间=" << std::chrono::system_clock::to_time_t(ride_request.get_earliest_departure()) << ", 最晚出发时间=" << std::chrono::system_clock::to_time_t(ride_request.get_latest_departure());
        return false;
    }

    // 验证状态是否合法
    if (static_cast<int>(ride_request.get_status()) < 0 || static_cast<int>(ride_request.get_status()) > 3) {
        LOG_ERROR << "行程请求状态不合法: 状态=" << static_cast<int>(ride_request.get_status());
        return false;
    }

    return true;
}

} // namespace service