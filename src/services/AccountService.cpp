#include "AccountService.h"
#include <iostream>
#include <algorithm>

namespace accounting {

AccountService::AccountService() {
}

bool AccountService::createAccount(const std::string& name, const std::string& type, double initial_balance, Account& new_account) {
    // 验证账户数据
    if (!validateAccountData(name, type)) {
        std::cerr << "Invalid account data." << std::endl;
        return false;
    }

    // 检查初始余额是否合法
    if (initial_balance < 0) {
        std::cerr << "Initial balance cannot be negative." << std::endl;
        return false;
    }

    // 创建账户对象
    Account account(0, name, type, initial_balance);

    // 调用 DAO 创建账户
    int new_account_id = 0;
    if (!account_dao_.createAccount(account, new_account_id)) {
        std::cerr << "Failed to create account." << std::endl;
        return false;
    }

    // 获取新创建的账户信息
    if (!account_dao_.getAccountById(new_account_id, new_account)) {
        std::cerr << "Failed to get new account." << std::endl;
        return false;
    }

    return true;
}

bool AccountService::getAccountById(int id, Account& account) {
    if (id <= 0) {
        std::cerr << "Invalid account ID." << std::endl;
        return false;
    }

    return account_dao_.getAccountById(id, account);
}

std::vector<Account> AccountService::getAllAccounts(const std::string& type_filter) {
    // 验证类型过滤条件
    if (!type_filter.empty()) {
        std::vector<std::string> valid_types = {"cash", "bank", "wallet"};
        if (std::find(valid_types.begin(), valid_types.end(), type_filter) == valid_types.end()) {
            std::cerr << "Invalid account type filter." << std::endl;
            return std::vector<Account>();
        }
    }

    return account_dao_.getAllAccounts(type_filter);
}

bool AccountService::updateAccount(int id, const std::string& name, const std::string& type) {
    if (id <= 0) {
        std::cerr << "Invalid account ID." << std::endl;
        return false;
    }

    // 验证账户数据
    if (!validateAccountData(name, type)) {
        std::cerr << "Invalid account data." << std::endl;
        return false;
    }

    // 检查账户是否存在
    Account existing_account;
    if (!account_dao_.getAccountById(id, existing_account)) {
        std::cerr << "Account not found." << std::endl;
        return false;
    }

    // 更新账户信息
    existing_account.setName(name);
    existing_account.setType(type);

    if (!account_dao_.updateAccount(existing_account)) {
        std::cerr << "Failed to update account." << std::endl;
        return false;
    }

    return true;
}

bool AccountService::deleteAccount(int id) {
    if (id <= 0) {
        std::cerr << "Invalid account ID." << std::endl;
        return false;
    }

    // 检查账户是否存在
    Account existing_account;
    if (!account_dao_.getAccountById(id, existing_account)) {
        std::cerr << "Account not found." << std::endl;
        return false;
    }

    // 调用 DAO 删除账户
    if (!account_dao_.deleteAccount(id)) {
        std::cerr << "Failed to delete account." << std::endl;
        return false;
    }

    return true;
}

bool AccountService::validateAccountData(const std::string& name, const std::string& type) {
    // 验证账户名称是否为空
    if (name.empty()) {
        std::cerr << "Account name cannot be empty." << std::endl;
        return false;
    }

    // 验证账户名称长度是否合法
    if (name.length() > 50) {
        std::cerr << "Account name cannot exceed 50 characters." << std::endl;
        return false;
    }

    // 验证账户类型是否合法
    std::vector<std::string> valid_types = {"cash", "bank", "wallet"};
    if (std::find(valid_types.begin(), valid_types.end(), type) == valid_types.end()) {
        std::cerr << "Invalid account type." << std::endl;
        return false;
    }

    return true;
}

} // namespace accounting
