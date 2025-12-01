#pragma once

#include "models/Account.h"
#include <vector>

namespace accounting {

class AccountDAO {
public:
    // 创建账户
    bool createAccount(const Account& account, int& new_account_id);

    // 根据 ID 查询账户
    bool getAccountById(int id, Account& account);

    // 查询所有账户
    std::vector<Account> getAllAccounts(const std::string& type_filter = "");

    // 更新账户
    bool updateAccount(const Account& account);

    // 删除账户
    bool deleteAccount(int id);
};

} // namespace accounting
