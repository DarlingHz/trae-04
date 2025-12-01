#include "service/TripService.h"
#include "utils/Logger.h"
#include <chrono>

namespace service {

TripService::TripService(std::shared_ptr<repository::TripRepository> trip_repo,
                         std::shared_ptr<repository::DriverRepository> driver_repo)
    : trip_repo_(trip_repo)
    , driver_repo_(driver_repo) {
}

TripService::~TripService() {
}

int TripService::create_trip(const model::Trip& trip) {
    LOG_INFO << "开始创建行程: 车主ID=" << trip.get_driver_id() << ", 乘客ID=" << trip.get_rider_id() << ", 行程请求ID=" << trip.get_ride_request_id();

    // 验证行程信息
    if (!validate_trip(trip)) {
        LOG_ERROR << "行程信息验证失败";
        return -1;
    }

    // 创建行程
    int trip_id = trip_repo_->create(trip);
    if (trip_id == -1) {
        LOG_ERROR << "行程创建失败";
        return -1;
    }

    LOG_INFO << "行程创建成功: ID=" << trip_id;
    return trip_id;
}

std::optional<model::Trip> TripService::get_trip_by_id(int id) {
    LOG_DEBUG << "获取行程信息: ID=" << id;

    auto trip = trip_repo_->get_by_id(id);
    if (trip) {
        LOG_DEBUG << "行程信息获取成功: ID=" << id << ", 车主ID=" << trip->get_driver_id() << ", 乘客ID=" << trip->get_rider_id() << ", 状态=" << static_cast<int>(trip->get_status());
    } else {
        LOG_DEBUG << "未找到行程: ID=" << id;
    }

    return trip;
}

std::vector<model::Trip> TripService::get_all_trips() {
    LOG_DEBUG << "获取所有行程信息";

    auto trips = trip_repo_->get_all();
    LOG_DEBUG << "获取行程信息成功: 总数=" << trips.size();

    return trips;
}

std::vector<model::Trip> TripService::get_trips_by_driver_id(int driver_id) {
    LOG_DEBUG << "根据车主ID获取行程信息: 车主ID=" << driver_id;

    auto trips = trip_repo_->get_by_driver_id(driver_id);
    LOG_DEBUG << "根据车主ID获取行程信息成功: 总数=" << trips.size();

    return trips;
}

std::vector<model::Trip> TripService::get_trips_by_rider_id(int rider_id) {
    LOG_DEBUG << "根据乘客ID获取行程信息: 乘客ID=" << rider_id;

    auto trips = trip_repo_->get_by_rider_id(rider_id);
    LOG_DEBUG << "根据乘客ID获取行程信息成功: 总数=" << trips.size();

    return trips;
}

std::optional<model::Trip> TripService::get_trip_by_ride_request_id(int ride_request_id) {
    LOG_DEBUG << "根据行程请求ID获取行程信息: 行程请求ID=" << ride_request_id;

    auto trip = trip_repo_->get_by_ride_request_id(ride_request_id);
    if (trip) {
        LOG_DEBUG << "根据行程请求ID获取行程信息成功: 行程ID=" << trip->get_id() << ", 车主ID=" << trip->get_driver_id() << ", 乘客ID=" << trip->get_rider_id() << ", 状态=" << static_cast<int>(trip->get_status());
    } else {
        LOG_DEBUG << "未找到对应的行程: 行程请求ID=" << ride_request_id;
    }

    return trip;
}

bool TripService::update_trip(const model::Trip& trip) {
    LOG_INFO << "更新行程信息: ID=" << trip.get_id();

    // 验证行程信息
    if (!validate_trip(trip)) {
        LOG_ERROR << "行程信息验证失败";
        return false;
    }

    // 检查行程是否存在
    auto existing_trip = trip_repo_->get_by_id(trip.get_id());
    if (!existing_trip) {
        LOG_ERROR << "行程不存在: ID=" << trip.get_id();
        return false;
    }

    // 更新行程信息
    if (!trip_repo_->update(trip)) {
        LOG_ERROR << "行程信息更新失败: ID=" << trip.get_id();
        return false;
    }

    LOG_INFO << "行程信息更新成功: ID=" << trip.get_id();
    return true;
}

bool TripService::update_trip_status(int id, model::TripStatus status) {
    LOG_INFO << "更新行程状态: ID=" << id << ", 新状态=" << static_cast<int>(status);

    // 检查行程是否存在
    auto existing_trip = trip_repo_->get_by_id(id);
    if (!existing_trip) {
        LOG_ERROR << "行程不存在: ID=" << id;
        return false;
    }

    // 更新行程状态
    if (!trip_repo_->update_status(id, status)) {
        LOG_ERROR << "行程状态更新失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程状态更新成功: ID=" << id;
    return true;
}

bool TripService::start_trip(int id) {
    LOG_INFO << "开始行程: ID=" << id;

    // 检查行程是否存在
    auto existing_trip = trip_repo_->get_by_id(id);
    if (!existing_trip) {
        LOG_ERROR << "行程不存在: ID=" << id;
        return false;
    }

    // 检查行程是否可以开始（只有进行中的行程可以开始）
    int current_status = static_cast<int>(existing_trip->get_status());
    if (current_status != static_cast<int>(model::TripStatus::ONGOING)) {
        LOG_ERROR << "行程无法开始: ID=" << id << ", 当前状态=" << current_status;
        return false;
    }

    // 更新行程开始时间和状态
    auto now = std::chrono::system_clock::now();
    if (!trip_repo_->update_start_time(id, now)) {
        LOG_ERROR << "行程开始时间更新失败: ID=" << id;
        return false;
    }

    if (!trip_repo_->update_status(id, model::TripStatus::ONGOING)) {
        LOG_ERROR << "行程状态更新失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程开始成功: ID=" << id;
    return true;
}

bool TripService::end_trip(int id, float fare) {
    LOG_INFO << "结束行程: ID=" << id << ", 费用=" << fare;

    // 检查行程是否存在
    auto existing_trip = trip_repo_->get_by_id(id);
    if (!existing_trip) {
        LOG_ERROR << "行程不存在: ID=" << id;
        return false;
    }

    // 检查行程是否可以结束（只有进行中的行程可以结束）
    int current_status = static_cast<int>(existing_trip->get_status());
    if (current_status != static_cast<int>(model::TripStatus::ONGOING)) {
        LOG_ERROR << "行程无法结束: ID=" << id << ", 当前状态=" << current_status;
        return false;
    }

    // 检查费用是否合法（必须大于等于0）
    if (fare < 0.0f) {
        LOG_ERROR << "行程费用不合法: 费用=" << fare;
        return false;
    }

    // 更新行程结束时间、费用和状态
    auto now = std::chrono::system_clock::now();
    if (!trip_repo_->update_end_time_and_fare(id, now, fare)) {
        LOG_ERROR << "行程结束时间和费用更新失败: ID=" << id;
        return false;
    }

    if (!trip_repo_->update_status(id, model::TripStatus::COMPLETED)) {
        LOG_ERROR << "行程状态更新失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程结束成功: ID=" << id;
    return true;
}

bool TripService::cancel_trip(int id) {
    LOG_INFO << "取消行程: ID=" << id;

    // 检查行程是否存在
    auto existing_trip = trip_repo_->get_by_id(id);
    if (!existing_trip) {
        LOG_ERROR << "行程不存在: ID=" << id;
        return false;
    }

    // 检查行程是否可以取消（只有进行中的行程可以取消）
    int current_status = static_cast<int>(existing_trip->get_status());
    if (current_status == static_cast<int>(model::TripStatus::COMPLETED) || 
        current_status == static_cast<int>(model::TripStatus::CANCELLED)) {
        LOG_ERROR << "行程无法取消: ID=" << id << ", 当前状态=" << current_status;
        return false;
    }

    // 更新行程状态为已取消
    if (!trip_repo_->update_status(id, model::TripStatus::CANCELLED)) {
        LOG_ERROR << "行程取消失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程取消成功: ID=" << id;
    return true;
}

bool TripService::delete_trip(int id) {
    LOG_INFO << "删除行程: ID=" << id;

    // 检查行程是否存在
    auto existing_trip = trip_repo_->get_by_id(id);
    if (!existing_trip) {
        LOG_ERROR << "行程不存在: ID=" << id;
        return false;
    }

    // 删除行程
    if (!trip_repo_->remove(id)) {
        LOG_ERROR << "行程删除失败: ID=" << id;
        return false;
    }

    LOG_INFO << "行程删除成功: ID=" << id;
    return true;
}

bool TripService::validate_trip(const model::Trip& trip) {
    // 验证车主ID是否合法
    if (trip.get_driver_id() <= 0) {
        LOG_ERROR << "车主ID不合法: 车主ID=" << trip.get_driver_id();
        return false;
    }

    // 验证乘客ID是否合法
    if (trip.get_rider_id() <= 0) {
        LOG_ERROR << "乘客ID不合法: 乘客ID=" << trip.get_rider_id();
        return false;
    }

    // 验证行程请求ID是否合法
    if (trip.get_ride_request_id() <= 0) {
        LOG_ERROR << "行程请求ID不合法: 行程请求ID=" << trip.get_ride_request_id();
        return false;
    }



    // 验证状态是否合法
    if (static_cast<int>(trip.get_status()) < 0 || static_cast<int>(trip.get_status()) > 4) {
        LOG_ERROR << "行程状态不合法: 状态=" << static_cast<int>(trip.get_status());
        return false;
    }

    return true;
}

} // namespace service