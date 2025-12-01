#include "BudgetController.h"

BudgetController::BudgetController() {
    budgetService = new accounting::BudgetService();
}

BudgetController::~BudgetController() {
    delete budgetService;
}

response<string_body> BudgetController::setBudget(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 解析请求体
        nlohmann::json requestBody = nlohmann::json::parse(req.body());

        // 验证必填字段
        if (!requestBody.contains("month") || !requestBody.contains("items")) {
            res.result(status::bad_request);
            nlohmann::json error;
            error["code"] = "INVALID_PARAM";
            error["message"] = "Missing required fields";
            res.body() = error.dump();
            res.prepare_payload();
            return res;
        }

        // 提取字段值
        std::string month = requestBody["month"];
        std::vector<accounting::BudgetItem> items;

        for (const auto& item : requestBody["items"]) {
            accounting::BudgetItem budgetItem;
            budgetItem.category_id = item["category_id"];
            budgetItem.limit = item["limit"];
            items.push_back(budgetItem);
        }

        // 设置预算
        budgetService->setBudget(month, items);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["message"] = "Budget set successfully";
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

response<string_body> BudgetController::getBudgets(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 提取查询参数
        std::string month;

        if (req.target().contains("?")) {
            std::string targetStr = std::string(req.target());
            std::string queryString = targetStr.substr(targetStr.find("?") + 1);
            std::vector<std::string> params;
            std::stringstream ss(queryString);
            std::string param;

            while (std::getline(ss, param, '&')) {
                params.push_back(param);
            }

            for (const auto& p : params) {
                if (p.substr(0, 6) == "month=") {
                    month = p.substr(6);
                }
            }
        }

        // 验证必填参数
        if (month.empty()) {
            res.result(status::bad_request);
            nlohmann::json error;
            error["code"] = "INVALID_PARAM";
            error["message"] = "Missing month parameter";
            res.body() = error.dump();
            res.prepare_payload();
            return res;
        }

        // 获取该月的预算
        std::vector<accounting::Budget> budgets = budgetService->getBudgetsByMonth(month);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["month"] = month;
        responseBody["budgets"] = nlohmann::json::array();

        for (const auto& budget : budgets) {
            nlohmann::json budgetJson;
            budgetJson["id"] = budget.getId();
            budgetJson["category_id"] = budget.getCategoryId();
            budgetJson["limit"] = budget.getLimit();
            responseBody["budgets"].push_back(budgetJson);
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
