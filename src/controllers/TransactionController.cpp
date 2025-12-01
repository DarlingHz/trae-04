#include "TransactionController.h"

TransactionController::TransactionController() {
    transactionService = new accounting::TransactionService();
}

TransactionController::~TransactionController() {
    delete transactionService;
}

response<string_body> TransactionController::createTransaction(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 解析请求体
        nlohmann::json requestBody = nlohmann::json::parse(req.body());

        // 验证必填字段
        if (!requestBody.contains("account_id") || !requestBody.contains("category_id") ||
            !requestBody.contains("type") || !requestBody.contains("amount") ||
            !requestBody.contains("time")) {
            res.result(status::bad_request);
            nlohmann::json error;
            error["code"] = "INVALID_PARAM";
            error["message"] = "Missing required fields";
            res.body() = error.dump();
            res.prepare_payload();
            return res;
        }

        // 提取字段值
        int account_id = requestBody["account_id"];
        int category_id = requestBody["category_id"];
        std::string type = requestBody["type"];
        double amount = requestBody["amount"];
        std::string time = requestBody["time"];
        std::string note = requestBody.contains("note") ? requestBody["note"] : "";

        // 创建交易记录
        accounting::Transaction newTransaction;
        bool createSuccess = transactionService->createTransaction(account_id, category_id, type, amount, time, note, newTransaction);

        // 构建响应
        nlohmann::json responseBody;
        if (createSuccess) {
            responseBody["id"] = newTransaction.getId();
            responseBody["account_id"] = newTransaction.getAccountId();
            responseBody["category_id"] = newTransaction.getCategoryId();
            responseBody["type"] = newTransaction.getType();
            responseBody["amount"] = newTransaction.getAmount();
            responseBody["time"] = newTransaction.getTime();
            responseBody["note"] = newTransaction.getNote();
        } else {
            responseBody["message"] = "Failed to create transaction";
        }
        res.body() = responseBody.dump();
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

response<string_body> TransactionController::getTransactions(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 提取查询参数
        accounting::TransactionFilter filter;
        int page = 1;
        int pageSize = 10;

        if (req.target().contains("?")) {
            std::string targetString = std::string(req.target());
            std::string queryString = targetString.substr(targetString.find("?") + 1);
            std::vector<std::string> params;
            std::stringstream ss(queryString);
            std::string param;

            while (std::getline(ss, param, '&')) {
                params.push_back(param);
            }

            for (const auto& p : params) {
                if (p.substr(0, 5) == "from=") {
                    filter.from_time = p.substr(5);
                } else if (p.substr(0, 3) == "to=") {
                    filter.to_time = p.substr(3);
                } else if (p.substr(0, 12) == "category_id=") {
                    filter.category_id = std::stoi(p.substr(12));
                } else if (p.substr(0, 11) == "account_id=") {
                    filter.account_id = std::stoi(p.substr(11));
                } else if (p.substr(0, 5) == "type=") {
                    filter.type = p.substr(5);
                } else if (p.substr(0, 11) == "amount_min=") {
                    filter.amount_min = std::stod(p.substr(11));
                } else if (p.substr(0, 11) == "amount_max=") {
                    filter.amount_max = std::stod(p.substr(11));
                } else if (p.substr(0, 5) == "page=") {
                    page = std::stoi(p.substr(5));
                } else if (p.substr(0, 10) == "page_size=") {
                    pageSize = std::stoi(p.substr(10));
                }
            }
        }

        // 获取交易记录列表
        accounting::TransactionPage transactionPage = transactionService->getTransactionsByPage(filter, page, pageSize);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["transactions"] = nlohmann::json::array();
        responseBody["total_count"] = transactionPage.total_count;
        responseBody["page"] = page;
        responseBody["page_size"] = pageSize;

        for (const auto& transaction : transactionPage.transactions) {
            nlohmann::json transactionJson;
            transactionJson["id"] = transaction.getId();
            transactionJson["account_id"] = transaction.getAccountId();
            transactionJson["category_id"] = transaction.getCategoryId();
            transactionJson["type"] = transaction.getType();
            transactionJson["amount"] = transaction.getAmount();
            transactionJson["time"] = transaction.getTime();
            transactionJson["note"] = transaction.getNote();
            responseBody["transactions"].push_back(transactionJson);
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

response<string_body> TransactionController::updateTransaction(request<string_body>& req, int id) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 解析请求体
        nlohmann::json requestBody = nlohmann::json::parse(req.body());

        // 提取字段值
        int account_id = -1;
        int category_id = -1;
        std::string type;
        double amount = -1.0;
        std::string time;
        std::string note;

        if (requestBody.contains("account_id")) {
            account_id = requestBody["account_id"];
        }

        if (requestBody.contains("category_id")) {
            category_id = requestBody["category_id"];
        }

        if (requestBody.contains("type")) {
            type = requestBody["type"];
        }

        if (requestBody.contains("amount")) {
            amount = requestBody["amount"];
        }

        if (requestBody.contains("time")) {
            time = requestBody["time"];
        }

        if (requestBody.contains("note")) {
            note = requestBody["note"];
        }

        // 更新交易记录
        bool updateSuccess = transactionService->updateTransaction(id, account_id, category_id, type, amount, time, note);

        // 构建响应
        nlohmann::json responseBody;
        if (updateSuccess) {
            responseBody["message"] = "Transaction updated successfully";
        } else {
            responseBody["message"] = "Failed to update transaction";
        }
        res.body() = responseBody.dump();
    } catch (const nlohmann::json::parse_error& e) {
        res.result(status::bad_request);
        nlohmann::json error;
        error["code"] = "INVALID_JSON";
        error["message"] = "Failed to parse JSON";
        res.body() = error.dump();
    } catch (const std::exception& e) {
        if (std::string(e.what()) == "Transaction not found") {
            res.result(status::not_found);
            nlohmann::json error;
            error["code"] = "NOT_FOUND";
            error["message"] = "Transaction not found";
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

response<string_body> TransactionController::deleteTransaction(request<string_body>& req, int id) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 删除交易记录
        transactionService->deleteTransaction(id);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["message"] = "Transaction deleted successfully";
        res.body() = responseBody.dump();
    } catch (const std::exception& e) {
        if (std::string(e.what()) == "Transaction not found") {
            res.result(status::not_found);
            nlohmann::json error;
            error["code"] = "NOT_FOUND";
            error["message"] = "Transaction not found";
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
