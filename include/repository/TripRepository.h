#ifndef RIDE_SHARING_BACKEND_TRIP_REPOSITORY_H
#define RIDE_SHARING_BACKEND_TRIP_REPOSITORY_H

#include <vector>
#include <optional>
#include "model/Trip.h"

namespace repository {

class TripRepository {
public:
    TripRepository() = default;
    virtual ~TripRepository() = default;

    // 创建行程
    virtual int create(const model::Trip& trip) = 0;

    // 根据ID获取行程
    virtual std::optional<model::Trip> get_by_id(int id) = 0;

    // 获取所有行程
    virtual std::vector<model::Trip> get_all() = 0;

    // 获取某个车主的行程
    virtual std::vector<model::Trip> get_by_driver_id(int driver_id) = 0;

    // 获取某个乘客的行程
    virtual std::vector<model::Trip> get_by_rider_id(int rider_id) = 0;

    // 获取某个出行请求的行程
    virtual std::optional<model::Trip> get_by_ride_request_id(int ride_request_id) = 0;

    // 更新行程
    virtual bool update(const model::Trip& trip) = 0;

    // 更新行程状态
    virtual bool update_status(int id, model::TripStatus status) = 0;

    // 更新行程开始时间
    virtual bool update_start_time(int id, const std::chrono::system_clock::time_point& start_time) = 0;

    // 更新行程结束时间和费用
    virtual bool update_end_time_and_fare(int id, const std::chrono::system_clock::time_point& end_time, float fare) = 0;

    // 删除行程
    virtual bool remove(int id) = 0;
};

class TripRepositoryImpl : public TripRepository {
public:
    TripRepositoryImpl() = default;
    ~TripRepositoryImpl() override = default;

    int create(const model::Trip& trip) override;
    std::optional<model::Trip> get_by_id(int id) override;
    std::vector<model::Trip> get_all() override;
    std::vector<model::Trip> get_by_driver_id(int driver_id) override;
    std::vector<model::Trip> get_by_rider_id(int rider_id) override;
    std::optional<model::Trip> get_by_ride_request_id(int ride_request_id) override;
    bool update(const model::Trip& trip) override;
    bool update_status(int id, model::TripStatus status) override;
    bool update_start_time(int id, const std::chrono::system_clock::time_point& start_time) override;
    bool update_end_time_and_fare(int id, const std::chrono::system_clock::time_point& end_time, float fare) override;
    bool remove(int id) override;
};

} // namespace repository

#endif //RIDE_SHARING_BACKEND_TRIP_REPOSITORY_H
