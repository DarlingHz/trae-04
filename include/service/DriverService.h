#ifndef DRIVER_SERVICE_H
#define DRIVER_SERVICE_H

#include <optional>
#include <vector>
#include <memory>
#include "model/Driver.h"
#include "repository/DriverRepository.h"

namespace service {

/**
 * 车主服务类，负责处理车主相关的业务逻辑
 */
class DriverService {
public:
    /**
     * 构造函数
     * @param driver_repo 车主数据访问层实例
     */
    explicit DriverService(std::shared_ptr<repository::DriverRepository> driver_repo);

    /**
     * 析构函数
     */
    ~DriverService();

    /**
     * 注册新车主
     * @param driver 车主信息
     * @return 注册成功后的车主ID（如果失败则返回-1）
     */
    int register_driver(const model::Driver& driver);

    /**
     * 根据ID获取车主信息
     * @param id 车主ID
     * @return 车主信息（如果未找到则返回std::nullopt）
     */
    std::optional<model::Driver> get_driver_by_id(int id);

    /**
     * 获取所有车主信息
     * @return 所有车主信息的列表
     */
    std::vector<model::Driver> get_all_drivers();

    /**
     * 获取所有可用的车主信息
     * @return 所有可用车主信息的列表
     */
    std::vector<model::Driver> get_available_drivers();

    /**
     * 更新车主信息
     * @param driver 车主信息
     * @return 更新是否成功
     */
    bool update_driver(const model::Driver& driver);

    /**
     * 更新车主状态
     * @param id 车主ID
     * @param status 新的状态
     * @return 更新是否成功
     */
    bool update_driver_status(int id, model::DriverStatus status);

    /**
     * 更新车主位置
     * @param id 车主ID
     * @param x X坐标
     * @param y Y坐标
     * @return 更新是否成功
     */
    bool update_driver_location(int id, int x, int y);

    /**
     * 删除车主
     * @param id 车主ID
     * @return 删除是否成功
     */
    bool delete_driver(int id);

private:
    /**
     * 验证车主信息
     * @param driver 车主信息
     * @return 验证是否通过
     */
    bool validate_driver(const model::Driver& driver);

    // 车主数据访问层实例
    std::shared_ptr<repository::DriverRepository> driver_repo_;
};

} // namespace service

#endif // DRIVER_SERVICE_H
