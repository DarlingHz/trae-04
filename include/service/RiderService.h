#ifndef RIDER_SERVICE_H
#define RIDER_SERVICE_H

#include <optional>
#include <vector>
#include <memory>
#include "model/Rider.h"
#include "repository/RiderRepository.h"

namespace service {

/**
 * 乘客服务类，负责处理乘客相关的业务逻辑
 */
class RiderService {
public:
    /**
     * 构造函数
     * @param rider_repo 乘客数据访问层实例
     */
    explicit RiderService(std::shared_ptr<repository::RiderRepository> rider_repo);

    /**
     * 析构函数
     */
    ~RiderService();

    /**
     * 注册新乘客
     * @param rider 乘客信息
     * @return 注册成功后的乘客ID（如果失败则返回-1）
     */
    int register_rider(const model::Rider& rider);

    /**
     * 根据ID获取乘客信息
     * @param id 乘客ID
     * @return 乘客信息（如果未找到则返回std::nullopt）
     */
    std::optional<model::Rider> get_rider_by_id(int id);

    /**
     * 获取所有乘客信息
     * @return 所有乘客信息的列表
     */
    std::vector<model::Rider> get_all_riders();

    /**
     * 更新乘客信息
     * @param rider 乘客信息
     * @return 更新是否成功
     */
    bool update_rider(const model::Rider& rider);

    /**
     * 删除乘客
     * @param id 乘客ID
     * @return 删除是否成功
     */
    bool delete_rider(int id);

private:
    /**
     * 验证乘客信息
     * @param rider 乘客信息
     * @return 验证是否通过
     */
    bool validate_rider(const model::Rider& rider);

    // 乘客数据访问层实例
    std::shared_ptr<repository::RiderRepository> rider_repo_;
};

} // namespace service

#endif // RIDER_SERVICE_H
