#ifndef RIDE_SHARING_BACKEND_TRIP_H
#define RIDE_SHARING_BACKEND_TRIP_H

#include <string>
#include <chrono>
#include <ostream>

namespace model {

enum class TripStatus {
    ONGOING,
    COMPLETED,
    CANCELLED
};

// 为TripStatus枚举类型添加ostream运算符重载
std::ostream& operator<<(std::ostream& os, const TripStatus& status);

class Trip {
public:
    Trip() = default;
    Trip(int id, int driver_id, int rider_id, int ride_request_id, 
         const std::chrono::system_clock::time_point& match_time, 
         const std::chrono::system_clock::time_point& start_time, 
         const std::chrono::system_clock::time_point& end_time, 
         TripStatus status, float fare);

    int get_id() const;
    void set_id(int id);

    int get_driver_id() const;
    void set_driver_id(int driver_id);

    int get_rider_id() const;
    void set_rider_id(int rider_id);

    int get_ride_request_id() const;
    void set_ride_request_id(int ride_request_id);

    const std::chrono::system_clock::time_point& get_match_time() const;
    void set_match_time(const std::chrono::system_clock::time_point& match_time);

    const std::chrono::system_clock::time_point& get_start_time() const;
    void set_start_time(const std::chrono::system_clock::time_point& start_time);

    const std::chrono::system_clock::time_point& get_end_time() const;
    void set_end_time(const std::chrono::system_clock::time_point& end_time);

    TripStatus get_status() const;
    void set_status(TripStatus status);

    float get_fare() const;
    void set_fare(float fare);

private:
    int id_ = 0;
    int driver_id_ = 0;
    int rider_id_ = 0;
    int ride_request_id_ = 0;
    std::chrono::system_clock::time_point match_time_;
    std::chrono::system_clock::time_point start_time_;
    std::chrono::system_clock::time_point end_time_;
    TripStatus status_ = TripStatus::ONGOING;
    float fare_ = 0.0f;
};

} // namespace model

#endif //RIDE_SHARING_BACKEND_TRIP_H