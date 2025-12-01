#ifndef RIDE_REQUEST_SERVICE_H
#define RIDE_REQUEST_SERVICE_H

#include <optional>
#include <vector>
#include <memory>
#include "model/RideRequest.h"
#include "repository/RideRequestRepository.h"
#include "service/MatchingService.h"

namespace service {

/**
 * 行程请求服务类，负责处理行程请求相关的业务逻辑
 */
class RideRequestService {
public:
    /**
     * 构造函数
     * @param ride_request_repo 行程请求数据访问层实例
     * @param matching_service 匹配服务实例
     */
    RideRequestService(std::shared_ptr<repository::RideRequestRepository> ride_request_repo,
                       MatchingService& matching_service);

    /**
     * 析构函数
     */
    ~RideRequestService();

    /**
     * 创建行程请求
     * @param ride_request 行程请求信息
     * @return 创建成功后的行程请求ID（如果失败则返回-1）
     */
    int create_ride_request(const model::RideRequest& ride_request);

    /**
     * 根据ID获取行程请求信息
     * @param id 行程请求ID
     * @return 行程请求信息（如果未找到则返回std::nullopt）
     */
    std::optional<model::RideRequest> get_ride_request_by_id(int id);

    /**
     * 获取所有行程请求信息
     * @return 所有行程请求信息的列表
     */
    std::vector<model::RideRequest> get_all_ride_requests();

    /**
     * 获取所有待匹配的行程请求信息
     * @return 所有待匹配行程请求信息的列表
     */
    std::vector<model::RideRequest> get_pending_ride_requests();

    /**
     * 根据乘客ID获取行程请求信息
     * @param rider_id 乘客ID
     * @return 该乘客的所有行程请求信息的列表
     */
    std::vector<model::RideRequest> get_ride_requests_by_rider_id(int rider_id);

    /**
     * 更新行程请求信息
     * @param ride_request 行程请求信息
     * @return 更新是否成功
     */
    bool update_ride_request(const model::RideRequest& ride_request);

    /**
     * 更新行程请求状态
     * @param id 行程请求ID
     * @param status 新的状态
     * @return 更新是否成功
     */
    bool update_ride_request_status(int id, model::RideRequestStatus status);

    /**
     * 取消行程请求
     * @param id 行程请求ID
     * @return 取消是否成功
     */
    bool cancel_ride_request(int id);

    /**
     * 删除行程请求
     * @param id 行程请求ID
     * @return 删除是否成功
     */
    bool delete_ride_request(int id);

private:
    /**
     * 验证行程请求信息
     * @param ride_request 行程请求信息
     * @return 验证是否通过
     */
    bool validate_ride_request(const model::RideRequest& ride_request);

    // 行程请求数据访问层实例
    std::shared_ptr<repository::RideRequestRepository> ride_request_repo_;

    // 匹配服务实例
    MatchingService& matching_service_;
};

} // namespace service

#endif // RIDE_REQUEST_SERVICE_H
