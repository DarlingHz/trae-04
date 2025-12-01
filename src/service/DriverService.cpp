#include "service/DriverService.h"
#include "utils/Logger.h"
#include <string>

namespace service {

DriverService::DriverService(std::shared_ptr<repository::DriverRepository> driver_repo)
    : driver_repo_(driver_repo) {
}

DriverService::~DriverService() {
}

int DriverService::register_driver(const model::Driver& driver) {
    LOG_INFO << "开始注册新车主: 姓名=" << driver.get_name() << ", 车牌号=" << driver.get_license_plate();

    // 验证车主信息
    if (!validate_driver(driver)) {
        LOG_ERROR << "车主信息验证失败"; 
        return -1;
    }

    // 检查车牌号是否已存在
    auto existing_drivers = driver_repo_->get_all();
    for (const auto& existing_driver : existing_drivers) {
        if (existing_driver.get_license_plate() == driver.get_license_plate()) {
            LOG_ERROR << "车牌号已存在: 车牌号=" << driver.get_license_plate();
            return -1;
        }
    }

    // 注册车主
    int driver_id = driver_repo_->create(driver);
    if (driver_id == -1) {
        LOG_ERROR << "车主注册失败"; 
        return -1;
    }

    LOG_INFO << "车主注册成功: ID=" << driver_id;
    return driver_id;
}

std::optional<model::Driver> DriverService::get_driver_by_id(int id) {
    LOG_DEBUG << "获取车主信息: ID=" << id;

    auto driver = driver_repo_->get_by_id(id);
    if (driver) {
        LOG_DEBUG << "车主信息获取成功: ID=" << id << ", 姓名=" << driver->get_name() << ", 车牌号=" << driver->get_license_plate();
    } else {
        LOG_DEBUG << "未找到车主: ID=" << id;
    }

    return driver;
}

std::vector<model::Driver> DriverService::get_all_drivers() {
    LOG_DEBUG << "获取所有车主信息"; 

    auto drivers = driver_repo_->get_all();
    LOG_DEBUG << "获取车主信息成功: 总数=" << drivers.size();

    return drivers;
}

std::vector<model::Driver> DriverService::get_available_drivers() {
    LOG_DEBUG << "获取所有可用车主信息"; 

    auto drivers = driver_repo_->get_available();
    LOG_DEBUG << "获取可用车主信息成功: 总数=" << drivers.size();

    return drivers;
}

bool DriverService::update_driver(const model::Driver& driver) {
    LOG_INFO << "更新车主信息: ID=" << driver.get_id();

    // 验证车主信息
    if (!validate_driver(driver)) {
        LOG_ERROR << "车主信息验证失败"; 
        return false;
    }

    // 检查车主是否存在
    auto existing_driver = driver_repo_->get_by_id(driver.get_id());
    if (!existing_driver) {
        LOG_ERROR << "车主不存在: ID=" << driver.get_id();
        return false;
    }

    // 检查车牌号是否已被其他车主使用
    auto all_drivers = driver_repo_->get_all();
    for (const auto& d : all_drivers) {
        if (d.get_id() != driver.get_id() && d.get_license_plate() == driver.get_license_plate()) {
            LOG_ERROR << "车牌号已被其他车主使用: 车牌号=" << driver.get_license_plate();
            return false;
        }
    }

    // 更新车主信息
    if (!driver_repo_->update(driver)) {
        LOG_ERROR << "车主信息更新失败: ID=" << driver.get_id();
        return false;
    }

    LOG_INFO << "车主信息更新成功: ID=" << driver.get_id();
    return true;
}

bool DriverService::update_driver_status(int id, model::DriverStatus status) {
    LOG_INFO << "更新车主状态: ID=" << id << ", 新状态=" << static_cast<int>(status);

    // 检查车主是否存在
    auto existing_driver = driver_repo_->get_by_id(id);
    if (!existing_driver) {
        LOG_ERROR << "车主不存在: ID=" << id;
        return false;
    }

    // 更新车主状态
    if (!driver_repo_->update_status(id, status)) {
        LOG_ERROR << "车主状态更新失败: ID=" << id;
        return false;
    }

    LOG_INFO << "车主状态更新成功: ID=" << id;
    return true;
}

bool DriverService::update_driver_location(int id, int x, int y) {
    LOG_INFO << "更新车主位置: ID=" << id << ", 新位置=({" << x << ", " << y << "})";

    // 检查车主是否存在
    auto existing_driver = driver_repo_->get_by_id(id);
    if (!existing_driver) {
        LOG_ERROR << "车主不存在: ID=" << id;
        return false;
    }

    // 更新车主位置
    if (!driver_repo_->update_location(id, x, y)) {
        LOG_ERROR << "车主位置更新失败: ID=" << id;
        return false;
    }

    LOG_INFO << "车主位置更新成功: ID=" << id;
    return true;
}

bool DriverService::delete_driver(int id) {
    LOG_INFO << "删除车主: ID=" << id;

    // 检查车主是否存在
    auto existing_driver = driver_repo_->get_by_id(id);
    if (!existing_driver) {
        LOG_ERROR << "车主不存在: ID=" << id;
        return false;
    }

    // 删除车主
    if (!driver_repo_->remove(id)) {
        LOG_ERROR << "车主删除失败: ID=" << id;
        return false;
    }

    LOG_INFO << "车主删除成功: ID=" << id;
    return true;
}

bool DriverService::validate_driver(const model::Driver& driver) {
    // 验证姓名是否为空
    if (driver.get_name().empty()) {
        LOG_ERROR << "车主姓名不能为空"; 
        return false;
    }

    // 验证姓名长度是否合法（1-50个字符）
    if (driver.get_name().length() < 1 || driver.get_name().length() > 50) {
        LOG_ERROR << "车主姓名长度不合法: 长度=" << driver.get_name().length();
        return false;
    }

    // 验证车牌号是否为空
    if (driver.get_license_plate().empty()) {
        LOG_ERROR << "车牌号不能为空"; 
        return false;
    }

    // 验证车牌号长度是否合法（1-20个字符）
    if (driver.get_license_plate().length() < 1 || driver.get_license_plate().length() > 20) {
        LOG_ERROR << "车牌号长度不合法: 长度=" << driver.get_license_plate().length();
        return false;
    }

    // 验证车型是否为空
    if (driver.get_car_model().empty()) {
        LOG_ERROR << "车型不能为空"; 
        return false;
    }

    // 验证车型长度是否合法（1-50个字符）
    if (driver.get_car_model().length() < 1 || driver.get_car_model().length() > 50) {
        LOG_ERROR << "车型长度不合法: 长度=" << driver.get_car_model().length();
        return false;
    }

    // 验证座位数是否合法（1-10个座位）
    if (driver.get_capacity() < 1 || driver.get_capacity() > 10) {
        LOG_ERROR << "座位数不合法: 座位数=" << driver.get_capacity();
        return false;
    }

    // 验证状态是否合法
    if (static_cast<int>(driver.get_status()) < 0 || static_cast<int>(driver.get_status()) > 2) {
        LOG_ERROR << "车主状态不合法: 状态=" << static_cast<int>(driver.get_status());
        return false;
    }

    // 验证评分是否合法（0.0-5.0）
    if (driver.get_rating() < 0.0f || driver.get_rating() > 5.0f) {
        LOG_ERROR << "车主评分不合法: 评分=" << driver.get_rating();
        return false;
    }

    return true;
}

} // namespace service
