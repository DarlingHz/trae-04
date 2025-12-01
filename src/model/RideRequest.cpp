#include "model/RideRequest.h"

namespace model {

RideRequest::RideRequest(int id, int rider_id, int start_x, int start_y, int end_x, int end_y, 
                            const std::chrono::system_clock::time_point& earliest_departure, 
                            const std::chrono::system_clock::time_point& latest_departure, 
                            RideRequestStatus status, const std::chrono::system_clock::time_point& create_time)
    : id_(id), rider_id_(rider_id), start_x_(start_x), start_y_(start_y), 
      end_x_(end_x), end_y_(end_y), earliest_departure_(earliest_departure), 
      latest_departure_(latest_departure), status_(status), create_time_(create_time) {}

int RideRequest::get_id() const {
    return id_;
}

void RideRequest::set_id(int id) {
    id_ = id;
}

int RideRequest::get_rider_id() const {
    return rider_id_;
}

void RideRequest::set_rider_id(int rider_id) {
    rider_id_ = rider_id;
}

int RideRequest::get_start_x() const {
    return start_x_;
}

void RideRequest::set_start_x(int start_x) {
    start_x_ = start_x;
}

int RideRequest::get_start_y() const {
    return start_y_;
}

void RideRequest::set_start_y(int start_y) {
    start_y_ = start_y;
}

int RideRequest::get_end_x() const {
    return end_x_;
}

void RideRequest::set_end_x(int end_x) {
    end_x_ = end_x;
}

int RideRequest::get_end_y() const {
    return end_y_;
}

void RideRequest::set_end_y(int end_y) {
    end_y_ = end_y;
}

const std::chrono::system_clock::time_point& RideRequest::get_earliest_departure() const {
    return earliest_departure_;
}

void RideRequest::set_earliest_departure(const std::chrono::system_clock::time_point& earliest_departure) {
    earliest_departure_ = earliest_departure;
}

const std::chrono::system_clock::time_point& RideRequest::get_latest_departure() const {
    return latest_departure_;
}

void RideRequest::set_latest_departure(const std::chrono::system_clock::time_point& latest_departure) {
    latest_departure_ = latest_departure;
}

RideRequestStatus RideRequest::get_status() const {
    return status_;
}

void RideRequest::set_status(RideRequestStatus status) {
    status_ = status;
}

const std::chrono::system_clock::time_point& RideRequest::get_create_time() const {
    return create_time_;
}

void RideRequest::set_create_time(const std::chrono::system_clock::time_point& create_time) {
    create_time_ = create_time;
}

} // namespace model
