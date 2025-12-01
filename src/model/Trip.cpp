#include "model/Trip.h"

namespace model {

// 为TripStatus枚举类型添加ostream运算符重载的实现
std::ostream& operator<<(std::ostream& os, const TripStatus& status) {
    switch (status) {
        case TripStatus::ONGOING:
            os << "ONGOING";
            break;
        case TripStatus::COMPLETED:
            os << "COMPLETED";
            break;
        case TripStatus::CANCELLED:
            os << "CANCELLED";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

Trip::Trip(int id, int driver_id, int rider_id, int ride_request_id, 
            const std::chrono::system_clock::time_point& match_time, 
            const std::chrono::system_clock::time_point& start_time, 
            const std::chrono::system_clock::time_point& end_time, 
            TripStatus status, float fare)
    : id_(id), driver_id_(driver_id), rider_id_(rider_id), ride_request_id_(ride_request_id), 
      match_time_(match_time), start_time_(start_time), end_time_(end_time), 
      status_(status), fare_(fare) {}

int Trip::get_id() const {
    return id_;
}

void Trip::set_id(int id) {
    id_ = id;
}

int Trip::get_driver_id() const {
    return driver_id_;
}

void Trip::set_driver_id(int driver_id) {
    driver_id_ = driver_id;
}

int Trip::get_rider_id() const {
    return rider_id_;
}

void Trip::set_rider_id(int rider_id) {
    rider_id_ = rider_id;
}

int Trip::get_ride_request_id() const {
    return ride_request_id_;
}

void Trip::set_ride_request_id(int ride_request_id) {
    ride_request_id_ = ride_request_id;
}

const std::chrono::system_clock::time_point& Trip::get_match_time() const {
    return match_time_;
}

void Trip::set_match_time(const std::chrono::system_clock::time_point& match_time) {
    match_time_ = match_time;
}

const std::chrono::system_clock::time_point& Trip::get_start_time() const {
    return start_time_;
}

void Trip::set_start_time(const std::chrono::system_clock::time_point& start_time) {
    start_time_ = start_time;
}

const std::chrono::system_clock::time_point& Trip::get_end_time() const {
    return end_time_;
}

void Trip::set_end_time(const std::chrono::system_clock::time_point& end_time) {
    end_time_ = end_time;
}

TripStatus Trip::get_status() const {
    return status_;
}

void Trip::set_status(TripStatus status) {
    status_ = status;
}

float Trip::get_fare() const {
    return fare_;
}

void Trip::set_fare(float fare) {
    fare_ = fare;
}

} // namespace model