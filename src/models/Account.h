#pragma once

#include <string>

namespace accounting {

class Account {
public:
    Account() = default;
    Account(int id, const std::string& name, const std::string& type, double initial_balance)
        : id_(id), name_(name), type_(type), initial_balance_(initial_balance) {}

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    const std::string& getType() const { return type_; }
    void setType(const std::string& type) { type_ = type; }

    double getInitialBalance() const { return initial_balance_; }
    void setInitialBalance(double initial_balance) { initial_balance_ = initial_balance; }

private:
    int id_ = 0;
    std::string name_;
    std::string type_;
    double initial_balance_ = 0.0;
};

} // namespace accounting
