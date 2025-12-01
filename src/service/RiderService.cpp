#include "service/RiderService.h"
#include "utils/Logger.h"
#include <string>

namespace service {

RiderService::RiderService(std::shared_ptr<repository::RiderRepository> rider_repo)
    : rider_repo_(rider_repo) {
}

RiderService::~RiderService() {
}

int RiderService::register_rider(const model::Rider& rider) {
    LOG_INFO << "开始注册新乘客: 姓名=" << rider.get_name();

    // 验证乘客信息
    if (!validate_rider(rider)) {
        LOG_ERROR << "乘客信息验证失败"; 
        return -1;
    }

    // 检查乘客是否已存在（根据手机号）
    auto existing_riders = rider_repo_->get_all();
    for (const auto& existing_rider : existing_riders) {
        if (existing_rider.get_phone() == rider.get_phone() && !rider.get_phone().empty()) {
            LOG_ERROR << "乘客已存在: 手机号=" << rider.get_phone();
            return -1;
        }
    }

    // 注册乘客
    int rider_id = rider_repo_->create(rider);
    if (rider_id == -1) {
        LOG_ERROR << "乘客注册失败"; 
        return -1;
    }

    LOG_INFO << "乘客注册成功: ID=" << rider_id;
    return rider_id;
}

std::optional<model::Rider> RiderService::get_rider_by_id(int id) {
    LOG_DEBUG << "获取乘客信息: ID=" << id;

    auto rider = rider_repo_->get_by_id(id);
    if (rider) {
        LOG_DEBUG << "乘客信息获取成功: ID=" << id << ", 姓名=" << rider->get_name();
    } else {
        LOG_DEBUG << "未找到乘客: ID=" << id;
    }

    return rider;
}

std::vector<model::Rider> RiderService::get_all_riders() {
    LOG_DEBUG << "获取所有乘客信息"; 

    auto riders = rider_repo_->get_all();
    LOG_DEBUG << "获取乘客信息成功: 总数=" << riders.size();

    return riders;
}

bool RiderService::update_rider(const model::Rider& rider) {
    LOG_INFO << "更新乘客信息: ID=" << rider.get_id();

    // 验证乘客信息
    if (!validate_rider(rider)) {
        LOG_ERROR << "乘客信息验证失败"; 
        return false;
    }

    // 检查乘客是否存在
    auto existing_rider = rider_repo_->get_by_id(rider.get_id());
    if (!existing_rider) {
        LOG_ERROR << "乘客不存在: ID=" << rider.get_id();
        return false;
    }

    // 检查手机号是否已被其他乘客使用
    auto all_riders = rider_repo_->get_all();
    for (const auto& r : all_riders) {
        if (r.get_id() != rider.get_id() && r.get_phone() == rider.get_phone() && !rider.get_phone().empty()) {
            LOG_ERROR << "手机号已被其他乘客使用: 手机号=" << rider.get_phone();
            return false;
        }
    }

    // 更新乘客信息
    if (!rider_repo_->update(rider)) {
        LOG_ERROR << "乘客信息更新失败: ID=" << rider.get_id();
        return false;
    }

    LOG_INFO << "乘客信息更新成功: ID=" << rider.get_id();
    return true;
}

bool RiderService::delete_rider(int id) {
    LOG_INFO << "删除乘客: ID=" << id;

    // 检查乘客是否存在
    auto existing_rider = rider_repo_->get_by_id(id);
    if (!existing_rider) {
        LOG_ERROR << "乘客不存在: ID=" << id;
        return false;
    }

    // 删除乘客
    if (!rider_repo_->remove(id)) {
        LOG_ERROR << "乘客删除失败: ID=" << id;
        return false;
    }

    LOG_INFO << "乘客删除成功: ID=" << id;
    return true;
}

bool RiderService::validate_rider(const model::Rider& rider) {
    // 验证姓名是否为空
    if (rider.get_name().empty()) {
        LOG_ERROR << "乘客姓名不能为空"; 
        return false;
    }

    // 验证姓名长度是否合法（1-50个字符）
    if (rider.get_name().length() < 1 || rider.get_name().length() > 50) {
        LOG_ERROR << "乘客姓名长度不合法: 长度=" << rider.get_name().length();
        return false;
    }

    // 验证手机号格式是否合法（可选，若提供则验证）
    if (!rider.get_phone().empty()) {
        // 简单的手机号格式验证：11位数字
        if (rider.get_phone().length() != 11) {
            LOG_ERROR << "手机号格式不合法: 长度=" << rider.get_phone().length();
            return false;
        }

        // 验证手机号是否全部由数字组成
        for (char c : rider.get_phone()) {
            if (!isdigit(c)) {
                LOG_ERROR << "手机号格式不合法: 包含非数字字符"; 
                return false;
            }
        }
    }

    // 验证评分是否合法（0.0-5.0）
    if (rider.get_rating() < 0.0f || rider.get_rating() > 5.0f) {
        LOG_ERROR << "乘客评分不合法: 评分=" << rider.get_rating();
        return false;
    }

    return true;
}

} // namespace service
