#ifndef RIDE_SHARING_BACKEND_DRIVER_REPOSITORY_H
#define RIDE_SHARING_BACKEND_DRIVER_REPOSITORY_H

#include <vector>
#include <optional>
#include "model/Driver.h"

namespace repository {

class DriverRepository {
public:
    DriverRepository() = default;
    virtual ~DriverRepository() = default;

    // 创建车主
    virtual int create(const model::Driver& driver) = 0;

    // 根据ID获取车主
    virtual std::optional<model::Driver> get_by_id(int id) = 0;

    // 获取所有车主
    virtual std::vector<model::Driver> get_all() = 0;

    // 获取可用车主
    virtual std::vector<model::Driver> get_available() = 0;

    // 更新车主信息
    virtual bool update(const model::Driver& driver) = 0;

    // 更新车主状态
    virtual bool update_status(int id, model::DriverStatus status) = 0;

    // 更新车主位置
    virtual bool update_location(int id, int x, int y) = 0;

    // 删除车主
    virtual bool remove(int id) = 0;
};

class DriverRepositoryImpl : public DriverRepository {
public:
    DriverRepositoryImpl() = default;
    ~DriverRepositoryImpl() override = default;

    int create(const model::Driver& driver) override;
    std::optional<model::Driver> get_by_id(int id) override;
    std::vector<model::Driver> get_all() override;
    std::vector<model::Driver> get_available() override;
    bool update(const model::Driver& driver) override;
    bool update_status(int id, model::DriverStatus status) override;
    bool update_location(int id, int x, int y) override;
    bool remove(int id) override;
};

} // namespace repository

#endif //RIDE_SHARING_BACKEND_DRIVER_REPOSITORY_H
