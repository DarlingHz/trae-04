#pragma once

#include <string>

namespace accounting {

class Category {
public:
    Category() = default;
    Category(int id, const std::string& name, const std::string& type)
        : id_(id), name_(name), type_(type) {}

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    const std::string& getType() const { return type_; }
    void setType(const std::string& type) { type_ = type; }

private:
    int id_ = 0;
    std::string name_;
    std::string type_; // "income" or "expense"
};

} // namespace accounting
