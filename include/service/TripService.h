#ifndef TRIP_SERVICE_H
#define TRIP_SERVICE_H

#include <optional>
#include <vector>
#include <memory>
#include "model/Trip.h"
#include "repository/TripRepository.h"
#include "repository/DriverRepository.h"

namespace service {

/**
 * 行程服务类，负责处理行程相关的业务逻辑
 */
class TripService {
public:
    /**
     * 构造函数
     * @param trip_repo 行程数据访问层实例
     * @param driver_repo 车主数据访问层实例
     */
    TripService(std::shared_ptr<repository::TripRepository> trip_repo,
               std::shared_ptr<repository::DriverRepository> driver_repo);

    /**
     * 析构函数
     */
    ~TripService();

    /**
     * 创建行程
     * @param trip 行程信息
     * @return 创建成功后的行程ID（如果失败则返回-1）
     */
    int create_trip(const model::Trip& trip);

    /**
     * 根据ID获取行程信息
     * @param id 行程ID
     * @return 行程信息（如果未找到则返回std::nullopt）
     */
    std::optional<model::Trip> get_trip_by_id(int id);

    /**
     * 获取所有行程信息
     * @return 所有行程信息的列表
     */
    std::vector<model::Trip> get_all_trips();

    /**
     * 根据车主ID获取行程信息
     * @param driver_id 车主ID
     * @return 该车主的所有行程信息的列表
     */
    std::vector<model::Trip> get_trips_by_driver_id(int driver_id);

    /**
     * 根据乘客ID获取行程信息
     * @param rider_id 乘客ID
     * @return 该乘客的所有行程信息的列表
     */
    std::vector<model::Trip> get_trips_by_rider_id(int rider_id);

    /**
     * 根据行程请求ID获取行程信息
     * @param ride_request_id 行程请求ID
     * @return 对应的行程信息（如果未找到则返回std::nullopt）
     */
    std::optional<model::Trip> get_trip_by_ride_request_id(int ride_request_id);

    /**
     * 更新行程信息
     * @param trip 行程信息
     * @return 更新是否成功
     */
    bool update_trip(const model::Trip& trip);

    /**
     * 更新行程状态
     * @param id 行程ID
     * @param status 新的状态
     * @return 更新是否成功
     */
    bool update_trip_status(int id, model::TripStatus status);

    /**
     * 开始行程
     * @param id 行程ID
     * @return 开始是否成功
     */
    bool start_trip(int id);

    /**
     * 结束行程
     * @param id 行程ID
     * @param fare 行程费用
     * @return 结束是否成功
     */
    bool end_trip(int id, float fare);

    /**
     * 取消行程
     * @param id 行程ID
     * @return 取消是否成功
     */
    bool cancel_trip(int id);

    /**
     * 删除行程
     * @param id 行程ID
     * @return 删除是否成功
     */
    bool delete_trip(int id);

private:
    /**
     * 验证行程信息
     * @param trip 行程信息
     * @return 验证是否通过
     */
    bool validate_trip(const model::Trip& trip);

    // 行程数据访问层实例
    std::shared_ptr<repository::TripRepository> trip_repo_;

    // 车主数据访问层实例
    std::shared_ptr<repository::DriverRepository> driver_repo_;
};

} // namespace service

#endif // TRIP_SERVICE_H
