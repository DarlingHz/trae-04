#include "model/Rider.h"

namespace model {

Rider::Rider(int id, const std::string& name, const std::string& phone, float rating, 
              const std::chrono::system_clock::time_point& registration_time)
    : id_(id), name_(name), phone_(phone), rating_(rating), registration_time_(registration_time) {}

int Rider::get_id() const {
    return id_;
}

void Rider::set_id(int id) {
    id_ = id;
}

const std::string& Rider::get_name() const {
    return name_;
}

void Rider::set_name(const std::string& name) {
    name_ = name;
}

const std::string& Rider::get_phone() const {
    return phone_;
}

void Rider::set_phone(const std::string& phone) {
    phone_ = phone;
}

float Rider::get_rating() const {
    return rating_;
}

void Rider::set_rating(float rating) {
    rating_ = rating;
}

const std::chrono::system_clock::time_point& Rider::get_registration_time() const {
    return registration_time_;
}

void Rider::set_registration_time(const std::chrono::system_clock::time_point& registration_time) {
    registration_time_ = registration_time;
}

} // namespace model
