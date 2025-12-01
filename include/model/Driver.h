#ifndef RIDE_SHARING_BACKEND_DRIVER_H
#define RIDE_SHARING_BACKEND_DRIVER_H

#include <string>
#include <chrono>

namespace model {

enum class DriverStatus {
    OFFLINE,
    AVAILABLE,
    ON_TRIP
};

class Driver {
public:
    Driver() = default;
    Driver(int id, const std::string& name, const std::string& license_plate, 
           const std::string& car_model, int capacity, DriverStatus status, 
           int current_x, int current_y, float rating, 
           const std::chrono::system_clock::time_point& registration_time);

    int get_id() const;
    void set_id(int id);

    const std::string& get_name() const;
    void set_name(const std::string& name);

    const std::string& get_license_plate() const;
    void set_license_plate(const std::string& license_plate);

    const std::string& get_car_model() const;
    void set_car_model(const std::string& car_model);

    int get_capacity() const;
    void set_capacity(int capacity);

    DriverStatus get_status() const;
    void set_status(DriverStatus status);

    int get_current_x() const;
    void set_current_x(int x);

    int get_current_y() const;
    void set_current_y(int y);

    float get_rating() const;
    void set_rating(float rating);

    const std::chrono::system_clock::time_point& get_registration_time() const;
    void set_registration_time(const std::chrono::system_clock::time_point& registration_time);

private:
    int id_ = 0;
    std::string name_;
    std::string license_plate_;
    std::string car_model_;
    int capacity_ = 4;
    DriverStatus status_ = DriverStatus::OFFLINE;
    int current_x_ = 0;
    int current_y_ = 0;
    float rating_ = 5.0f;
    std::chrono::system_clock::time_point registration_time_;
};

} // namespace model

#endif //RIDE_SHARING_BACKEND_DRIVER_H
