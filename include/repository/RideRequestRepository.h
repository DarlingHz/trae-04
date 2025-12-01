#ifndef RIDE_SHARING_BACKEND_RIDE_REQUEST_REPOSITORY_H
#define RIDE_SHARING_BACKEND_RIDE_REQUEST_REPOSITORY_H

#include <vector>
#include <optional>
#include "model/RideRequest.h"

namespace repository {

class RideRequestRepository {
public:
    RideRequestRepository() = default;
    virtual ~RideRequestRepository() = default;

    // 创建出行请求
    virtual int create(const model::RideRequest& ride_request) = 0;

    // 根据ID获取出行请求
    virtual std::optional<model::RideRequest> get_by_id(int id) = 0;

    // 获取所有出行请求
    virtual std::vector<model::RideRequest> get_all() = 0;

    // 获取待匹配的出行请求
    virtual std::vector<model::RideRequest> get_pending() = 0;

    // 获取某个乘客的出行请求
    virtual std::vector<model::RideRequest> get_by_rider_id(int rider_id) = 0;

    // 更新出行请求
    virtual bool update(const model::RideRequest& ride_request) = 0;

    // 更新出行请求状态
    virtual bool update_status(int id, model::RideRequestStatus status) = 0;

    // 删除出行请求
    virtual bool remove(int id) = 0;
};

class RideRequestRepositoryImpl : public RideRequestRepository {
public:
    RideRequestRepositoryImpl() = default;
    ~RideRequestRepositoryImpl() override = default;

    int create(const model::RideRequest& ride_request) override;
    std::optional<model::RideRequest> get_by_id(int id) override;
    std::vector<model::RideRequest> get_all() override;
    std::vector<model::RideRequest> get_pending() override;
    std::vector<model::RideRequest> get_by_rider_id(int rider_id) override;
    bool update(const model::RideRequest& ride_request) override;
    bool update_status(int id, model::RideRequestStatus status) override;
    bool remove(int id) override;
};

} // namespace repository

#endif //RIDE_SHARING_BACKEND_RIDE_REQUEST_REPOSITORY_H
