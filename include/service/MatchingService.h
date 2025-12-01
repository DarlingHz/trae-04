#ifndef MATCHING_SERVICE_H
#define MATCHING_SERVICE_H

#include <vector>
#include <optional>
#include <mutex>
#include <unordered_map>
#include <memory>
#include "model/Driver.h"
#include "model/RideRequest.h"
#include "repository/DriverRepository.h"
#include "repository/RideRequestRepository.h"
#include "repository/TripRepository.h"

namespace service {

/**
 * 匹配服务类，负责乘客出行请求与车主的匹配逻辑
 * 采用单例模式，确保全局唯一实例
 */
class MatchingService {
public:
    /**
     * 获取单例实例
     * @return 匹配服务实例
     */
    static MatchingService& get_instance();

    /**
     * 初始化匹配服务
     * @param driver_repo 车主数据访问层实例
     * @param ride_request_repo 行程请求数据访问层实例
     * @param trip_repo 行程数据访问层实例
     */
    void init(std::shared_ptr<repository::DriverRepository> driver_repo,
              std::shared_ptr<repository::RideRequestRepository> ride_request_repo,
              std::shared_ptr<repository::TripRepository> trip_repo);

    /**
     * 触发匹配过程
     * 当有新的行程请求或车主状态/位置更新时调用
     */
    void trigger_matching();

    /**
     * 当有新的行程请求时调用
     * @param ride_request 新的行程请求
     */
    void on_new_ride_request(const model::RideRequest& ride_request);

    /**
     * 当车主状态更新时调用
     * @param driver 状态更新后的车主
     */
    void on_driver_status_update(const model::Driver& driver);

    /**
     * 当车主位置更新时调用
     * @param driver 位置更新后的车主
     */
    void on_driver_location_update(const model::Driver& driver);

    /**
     * 获取匹配统计信息
     * @return 匹配统计信息的字符串表示
     */
    std::string get_matching_stats();

private:
    /**
     * 构造函数（私有，防止外部实例化）
     */
    MatchingService();

    /**
     * 析构函数（公共，允许外部删除）
     */
    ~MatchingService();

    /**
     * 匹配算法核心逻辑
     * @param pending_requests 待匹配的行程请求列表
     * @param available_drivers 可用的车主列表
     */
    void perform_matching(const std::vector<model::RideRequest>& pending_requests,
                           const std::vector<model::Driver>& available_drivers);

    /**
     * 计算两个点之间的曼哈顿距离
     * @param x1 第一个点的X坐标
     * @param y1 第一个点的Y坐标
     * @param x2 第二个点的X坐标
     * @param y2 第二个点的Y坐标
     * @return 曼哈顿距离
     */
    int calculate_manhattan_distance(int x1, int y1, int x2, int y2);

    /**
     * 计算两个点之间的欧几里得距离
     * @param x1 第一个点的X坐标
     * @param y1 第一个点的Y坐标
     * @param x2 第二个点的X坐标
     * @param y2 第二个点的Y坐标
     * @return 欧几里得距离的平方（避免开方运算，提高性能）
     */
    int calculate_euclidean_distance_squared(int x1, int y1, int x2, int y2);

    /**
     * 选择最佳匹配的车主
     * @param request 行程请求
     * @param available_drivers 可用的车主列表
     * @return 最佳匹配的车主（如果找到）
     */
    std::optional<model::Driver> select_best_match(const model::RideRequest& request,
                                                       const std::vector<model::Driver>& available_drivers);

    /**
     * 完成匹配操作
     * @param request 行程请求
     * @param driver 匹配的车主
     * @return 匹配是否成功
     */
    bool complete_matching(const model::RideRequest& request, const model::Driver& driver);

    // 数据访问层实例
    std::shared_ptr<repository::DriverRepository> driver_repo_;
    std::shared_ptr<repository::RideRequestRepository> ride_request_repo_;
    std::shared_ptr<repository::TripRepository> trip_repo_;

    // 匹配统计信息
    int total_matching_attempts_; // 总匹配尝试次数
    int successful_matches_; // 成功匹配次数
    int total_pending_requests_; // 待匹配请求总数
    int total_available_drivers_; // 可用车主总数

    // 互斥锁，保护匹配过程的线程安全
    std::mutex matching_mutex_;

    // 匹配半径阈值（曼哈顿距离）
    static const int MATCHING_RADIUS_THRESHOLD = 100;
};

} // namespace service

#endif // MATCHING_SERVICE_H
