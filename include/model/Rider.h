#ifndef RIDE_SHARING_BACKEND_RIDER_H
#define RIDE_SHARING_BACKEND_RIDER_H

#include <string>
#include <chrono>

namespace model {

class Rider {
public:
    Rider() = default;
    Rider(int id, const std::string& name, const std::string& phone, float rating, 
          const std::chrono::system_clock::time_point& registration_time);

    int get_id() const;
    void set_id(int id);

    const std::string& get_name() const;
    void set_name(const std::string& name);

    const std::string& get_phone() const;
    void set_phone(const std::string& phone);

    float get_rating() const;
    void set_rating(float rating);

    const std::chrono::system_clock::time_point& get_registration_time() const;
    void set_registration_time(const std::chrono::system_clock::time_point& registration_time);

private:
    int id_ = 0;
    std::string name_;
    std::string phone_;
    float rating_ = 5.0f;
    std::chrono::system_clock::time_point registration_time_;
};

} // namespace model

#endif //RIDE_SHARING_BACKEND_RIDER_H
