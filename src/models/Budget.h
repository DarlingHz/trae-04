#pragma once

#include <string>

namespace accounting {

class Budget {
public:
    Budget() = default;
    Budget(int id, const std::string& month, int category_id, double limit)
        : id_(id), month_(month), category_id_(category_id), limit_(limit) {}

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    const std::string& getMonth() const { return month_; }
    void setMonth(const std::string& month) { month_ = month; }

    int getCategoryId() const { return category_id_; }
    void setCategoryId(int category_id) { category_id_ = category_id; }

    double getLimit() const { return limit_; }
    void setLimit(double limit) { limit_ = limit; }

private:
    int id_ = 0;
    std::string month_; // 格式如 "2024-11"
    int category_id_ = 0;
    double limit_ = 0.0;
};

} // namespace accounting
