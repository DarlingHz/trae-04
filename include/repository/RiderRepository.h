#ifndef RIDE_SHARING_BACKEND_RIDER_REPOSITORY_H
#define RIDE_SHARING_BACKEND_RIDER_REPOSITORY_H

#include <vector>
#include <optional>
#include "model/Rider.h"

namespace repository {

class RiderRepository {
public:
    RiderRepository() = default;
    virtual ~RiderRepository() = default;

    // 创建乘客
    virtual int create(const model::Rider& rider) = 0;

    // 根据ID获取乘客
    virtual std::optional<model::Rider> get_by_id(int id) = 0;

    // 获取所有乘客
    virtual std::vector<model::Rider> get_all() = 0;

    // 更新乘客信息
    virtual bool update(const model::Rider& rider) = 0;

    // 删除乘客
    virtual bool remove(int id) = 0;
};

class RiderRepositoryImpl : public RiderRepository {
public:
    RiderRepositoryImpl() = default;
    ~RiderRepositoryImpl() override = default;

    int create(const model::Rider& rider) override;
    std::optional<model::Rider> get_by_id(int id) override;
    std::vector<model::Rider> get_all() override;
    bool update(const model::Rider& rider) override;
    bool remove(int id) override;
};

} // namespace repository

#endif //RIDE_SHARING_BACKEND_RIDER_REPOSITORY_H
