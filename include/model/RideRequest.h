#ifndef RIDE_SHARING_BACKEND_RIDE_REQUEST_H
#define RIDE_SHARING_BACKEND_RIDE_REQUEST_H

#include <string>
#include <chrono>

namespace model {

enum class RideRequestStatus {
    PENDING,
    MATCHED,
    CANCELLED,
    COMPLETED
};

class RideRequest {
public:
    RideRequest() = default;
    RideRequest(int id, int rider_id, int start_x, int start_y, int end_x, int end_y, 
                const std::chrono::system_clock::time_point& earliest_departure, 
                const std::chrono::system_clock::time_point& latest_departure, 
                RideRequestStatus status, const std::chrono::system_clock::time_point& create_time);

    int get_id() const;
    void set_id(int id);

    int get_rider_id() const;
    void set_rider_id(int rider_id);

    int get_start_x() const;
    void set_start_x(int start_x);

    int get_start_y() const;
    void set_start_y(int start_y);

    int get_end_x() const;
    void set_end_x(int end_x);

    int get_end_y() const;
    void set_end_y(int end_y);

    const std::chrono::system_clock::time_point& get_earliest_departure() const;
    void set_earliest_departure(const std::chrono::system_clock::time_point& earliest_departure);

    const std::chrono::system_clock::time_point& get_latest_departure() const;
    void set_latest_departure(const std::chrono::system_clock::time_point& latest_departure);

    RideRequestStatus get_status() const;
    void set_status(RideRequestStatus status);

    const std::chrono::system_clock::time_point& get_create_time() const;
    void set_create_time(const std::chrono::system_clock::time_point& create_time);

private:
    int id_ = 0;
    int rider_id_ = 0;
    int start_x_ = 0;
    int start_y_ = 0;
    int end_x_ = 0;
    int end_y_ = 0;
    std::chrono::system_clock::time_point earliest_departure_;
    std::chrono::system_clock::time_point latest_departure_;
    RideRequestStatus status_ = RideRequestStatus::PENDING;
    std::chrono::system_clock::time_point create_time_;
};

} // namespace model

#endif //RIDE_SHARING_BACKEND_RIDE_REQUEST_H
