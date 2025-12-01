#pragma once

#include <string>

namespace accounting {

class Transaction {
public:
    Transaction() = default;
    Transaction(int id, int account_id, int category_id, const std::string& type,
                double amount, const std::string& time, const std::string& note)
        : id_(id), account_id_(account_id), category_id_(category_id), type_(type),
          amount_(amount), time_(time), note_(note) {}

    int getId() const { return id_; }
    void setId(int id) { id_ = id; }

    int getAccountId() const { return account_id_; }
    void setAccountId(int account_id) { account_id_ = account_id; }

    int getCategoryId() const { return category_id_; }
    void setCategoryId(int category_id) { category_id_ = category_id; }

    const std::string& getType() const { return type_; }
    void setType(const std::string& type) { type_ = type; }

    double getAmount() const { return amount_; }
    void setAmount(double amount) { amount_ = amount; }

    const std::string& getTime() const { return time_; }
    void setTime(const std::string& time) { time_ = time; }

    const std::string& getNote() const { return note_; }
    void setNote(const std::string& note) { note_ = note; }

private:
    int id_ = 0;
    int account_id_ = 0;
    int category_id_ = 0;
    std::string type_; // "income" or "expense"
    double amount_ = 0.0;
    std::string time_;
    std::string note_;
};

} // namespace accounting
