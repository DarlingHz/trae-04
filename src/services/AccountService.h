#pragma once

#include "dao/AccountDAO.h"
#include "models/Account.h"
#include <vector>
#include <string>

namespace accounting {

class AccountService {
public:
    AccountService();
    ~AccountService() = default;

    // 创建账户
    bool createAccount(const std::string& name, const std::string& type, double initial_balance, Account& new_account);

    // 根据 ID 查询账户
    bool getAccountById(int id, Account& account);

    // 查询所有账户
    std::vector<Account> getAllAccounts(const std::string& type_filter = "");

    // 更新账户
    bool updateAccount(int id, const std::string& name, const std::string& type);

    // 删除账户
    bool deleteAccount(int id);

private:
    AccountDAO account_dao_;

    // 验证账户数据
    bool validateAccountData(const std::string& name, const std::string& type);
};

} // namespace accounting
