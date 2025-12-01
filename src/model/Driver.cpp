#include "model/Driver.h"

namespace model {

Driver::Driver(int id, const std::string& name, const std::string& license_plate, 
               const std::string& car_model, int capacity, DriverStatus status, 
               int current_x, int current_y, float rating, 
               const std::chrono::system_clock::time_point& registration_time)
    : id_(id), name_(name), license_plate_(license_plate), car_model_(car_model), 
      capacity_(capacity), status_(status), current_x_(current_x), current_y_(current_y), 
      rating_(rating), registration_time_(registration_time) {}

int Driver::get_id() const {
    return id_;
}

void Driver::set_id(int id) {
    id_ = id;
}

const std::string& Driver::get_name() const {
    return name_;
}

void Driver::set_name(const std::string& name) {
    name_ = name;
}

const std::string& Driver::get_license_plate() const {
    return license_plate_;
}

void Driver::set_license_plate(const std::string& license_plate) {
    license_plate_ = license_plate;
}

const std::string& Driver::get_car_model() const {
    return car_model_;
}

void Driver::set_car_model(const std::string& car_model) {
    car_model_ = car_model;
}

int Driver::get_capacity() const {
    return capacity_;
}

void Driver::set_capacity(int capacity) {
    capacity_ = capacity;
}

DriverStatus Driver::get_status() const {
    return status_;
}

void Driver::set_status(DriverStatus status) {
    status_ = status;
}

int Driver::get_current_x() const {
    return current_x_;
}

void Driver::set_current_x(int x) {
    current_x_ = x;
}

int Driver::get_current_y() const {
    return current_y_;
}

void Driver::set_current_y(int y) {
    current_y_ = y;
}

float Driver::get_rating() const {
    return rating_;
}

void Driver::set_rating(float rating) {
    rating_ = rating;
}

const std::chrono::system_clock::time_point& Driver::get_registration_time() const {
    return registration_time_;
}

void Driver::set_registration_time(const std::chrono::system_clock::time_point& registration_time) {
    registration_time_ = registration_time;
}

} // namespace model
