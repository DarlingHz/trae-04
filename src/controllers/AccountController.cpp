#include "AccountController.h"

AccountController::AccountController() {
    accountService = new accounting::AccountService();
}

AccountController::~AccountController() {
    delete accountService;
}

response<string_body> AccountController::createAccount(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 解析请求体
        json requestBody = json::parse(req.body());

        // 验证必填字段
        if (!requestBody.contains("name") || !requestBody.contains("type")) {
            res.result(status::bad_request);
            nlohmann::json error;
            error["code"] = "INVALID_PARAM";
            error["message"] = "Missing required fields";
            res.body() = error.dump();
            res.prepare_payload();
            return res;
        }

        // 提取字段值
        std::string name = requestBody["name"];
        std::string type = requestBody["type"];
        double initial_balance = 0.0;
        if (requestBody.contains("initial_balance")) {
            initial_balance = requestBody["initial_balance"].get<double>();
        }

        // 创建账户
        accounting::Account account;
        bool success = accountService->createAccount(name, type, initial_balance, account);

        if (success) {
            // 构建响应
            nlohmann::json responseBody;
            responseBody["id"] = account.getId();
            responseBody["name"] = account.getName();
            responseBody["type"] = account.getType();
            responseBody["initial_balance"] = account.getInitialBalance();
            res.body() = responseBody.dump();
        } else {
            res.result(status::internal_server_error);
            nlohmann::json error;
            error["code"] = "INTERNAL_ERROR";
            error["message"] = "Failed to create account";
            res.body() = error.dump();
        }
    } catch (const nlohmann::json::parse_error& e) {
        res.result(status::bad_request);
        nlohmann::json error;
        error["code"] = "INVALID_JSON";
        error["message"] = "Failed to parse JSON";
        res.body() = error.dump();
    } catch (const std::exception& e) {
        res.result(status::internal_server_error);
        nlohmann::json error;
        error["code"] = "INTERNAL_ERROR";
        error["message"] = e.what();
        res.body() = error.dump();
    }

    res.prepare_payload();
    return res;
}

response<string_body> AccountController::getAccounts(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 提取查询参数
        std::string typeFilter;
        int page = 1;
        int pageSize = 10;

        if (req.target().contains("?")) {
            std::string target = std::string(req.target());
            std::string queryString = target.substr(target.find("?") + 1);
            std::vector<std::string> params;
            std::stringstream ss(queryString);
            std::string param;

            while (std::getline(ss, param, '&')) {
                params.push_back(param);
            }

            for (const auto& p : params) {
                if (p.substr(0, 5) == "type=") {
                    typeFilter = p.substr(5);
                } else if (p.substr(0, 5) == "page=") {
                    page = std::stoi(p.substr(5));
                } else if (p.substr(0, 10) == "page_size=") {
                    pageSize = std::stoi(p.substr(10));
                }
            }
        }

        // 获取账户列表
        std::vector<accounting::Account> accounts = accountService->getAllAccounts();

        // 应用过滤
        if (!typeFilter.empty()) {
            std::vector<accounting::Account> filteredAccounts;
            for (const auto& account : accounts) {
                if (account.getType() == typeFilter) {
                    filteredAccounts.push_back(account);
                }
            }
            accounts = filteredAccounts;
        }

        // 应用分页
        int startIndex = (page - 1) * pageSize;
        int endIndex = std::min(startIndex + pageSize, (int)accounts.size());
        std::vector<accounting::Account> paginatedAccounts(accounts.begin() + startIndex, accounts.begin() + endIndex);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["accounts"] = nlohmann::json::array();
        responseBody["total_count"] = accounts.size();
        responseBody["page"] = page;
        responseBody["page_size"] = pageSize;

        for (const auto& account : paginatedAccounts) {
            nlohmann::json accountJson;
            accountJson["id"] = account.getId();
            accountJson["name"] = account.getName();
            accountJson["type"] = account.getType();
            accountJson["initial_balance"] = account.getInitialBalance();
            responseBody["accounts"].push_back(accountJson);
        }

        res.body() = responseBody.dump();
    } catch (const std::exception& e) {
        res.result(status::internal_server_error);
        nlohmann::json error;
        error["code"] = "INTERNAL_ERROR";
        error["message"] = e.what();
        res.body() = error.dump();
    }

    res.prepare_payload();
    return res;
}

response<string_body> AccountController::updateAccount(request<string_body>& req, int id) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 解析请求体
        json requestBody = json::parse(req.body());

        // 提取字段值
        std::string name;
        std::string type;

        if (requestBody.contains("name")) {
            name = requestBody["name"];
        }

        if (requestBody.contains("type")) {
            type = requestBody["type"];
        }

        // 更新账户
        bool success = accountService->updateAccount(id, name, type);

        if (success) {
            // 获取更新后的账户信息
            accounting::Account account;
            accountService->getAccountById(id, account);

            // 构建响应
            json responseBody = {
                {"id", account.getId()},
                {"name", account.getName()},
                {"type", account.getType()},
                {"initial_balance", account.getInitialBalance()}
            };
            res.body() = responseBody.dump();
        } else {
            res.result(status::not_found);
            nlohmann::json error;
            error["code"] = "NOT_FOUND";
            error["message"] = "Account not found";
            res.body() = error.dump();
        }
    } catch (const nlohmann::json::parse_error& e) {
        res.result(status::bad_request);
        nlohmann::json error;
        error["code"] = "INVALID_JSON";
        error["message"] = "Failed to parse JSON";
        res.body() = error.dump();
    } catch (const std::exception& e) {
        res.result(status::internal_server_error);
        nlohmann::json error;
        error["code"] = "INTERNAL_ERROR";
        error["message"] = e.what();
        res.body() = error.dump();
    }

    res.prepare_payload();
    return res;
}

response<string_body> AccountController::deleteAccount(int id) {
    response<string_body> res(status::ok, 11);
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 删除账户
        accountService->deleteAccount(id);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["message"] = "Account deleted successfully";
        res.body() = responseBody.dump();
    } catch (const std::exception& e) {
        if (std::string(e.what()) == "Account not found") {
            res.result(status::not_found);
            nlohmann::json error;
            error["code"] = "NOT_FOUND";
            error["message"] = "Account not found";
            res.body() = error.dump();
        } else {
            res.result(status::internal_server_error);
            nlohmann::json error;
            error["code"] = "INTERNAL_ERROR";
            error["message"] = e.what();
            res.body() = error.dump();
        }
    }

    res.prepare_payload();
    return res;
}
