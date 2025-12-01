#ifndef USER_H
#define USER_H

#include <string>
#include <chrono>

namespace model {

class User {
public:
    User() = default;
    User(int id, std::string nickname, std::chrono::system_clock::time_point created_at, std::chrono::system_clock::time_point updated_at)
        : id_(id), nickname_(std::move(nickname)), created_at_(created_at), updated_at_(updated_at) {}

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    const std::string& getNickname() const { return nickname_; }
    void setNickname(std::string nickname) { nickname_ = std::move(nickname); }

    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    void setCreatedAt(std::chrono::system_clock::time_point created_at) { created_at_ = created_at; }

    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }
    void setUpdatedAt(std::chrono::system_clock::time_point updated_at) { updated_at_ = updated_at; }

private:
    int id_ = 0;
    std::string nickname_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
};

} // namespace model

#endif // USER_H
