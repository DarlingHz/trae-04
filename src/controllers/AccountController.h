#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "services/AccountService.h"

using namespace boost::beast;
using namespace boost::beast::http;
using json = nlohmann::json;

class AccountController {
public:
    AccountController();
    ~AccountController();

    response<string_body> createAccount(request<string_body>& req);
    response<string_body> getAccounts(request<string_body>& req);
    response<string_body> updateAccount(request<string_body>& req, int id);
    response<string_body> deleteAccount(int id);

private:
    accounting::AccountService* accountService;
};
