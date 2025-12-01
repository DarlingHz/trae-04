#include "SummaryController.h"

SummaryController::SummaryController() {
    summaryService = new accounting::SummaryService();
}

SummaryController::~SummaryController() {
    delete summaryService;
}

response<string_body> SummaryController::getMonthlySummary(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 提取查询参数
        std::string month;

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

        // 获取月度汇总
        accounting::MonthlySummary summary = summaryService->getMonthlySummary(month);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["month"] = summary.month;
        responseBody["total_income"] = summary.total_income;
        responseBody["total_expense"] = summary.total_expense;
        responseBody["balance"] = summary.balance;
        responseBody["per_category"] = nlohmann::json::array();

        for (const auto& categorySummary : summary.per_category) {
            nlohmann::json categoryJson;
            categoryJson["category_id"] = categorySummary.category_id;
            categoryJson["category_name"] = categorySummary.category_name;
            categoryJson["expense"] = categorySummary.expense;
            categoryJson["budget_limit"] = categorySummary.budget_limit;
            categoryJson["exceed"] = categorySummary.exceed;
            responseBody["per_category"].push_back(categoryJson);
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

response<string_body> SummaryController::getTrendSummary(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 提取查询参数
        std::string from;
        std::string to;

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
                    from = p.substr(5);
                } else if (p.substr(0, 3) == "to=") {
                    to = p.substr(3);
                }
            }
        }

        // 验证必填参数
        if (from.empty() || to.empty()) {
            res.result(status::bad_request);
            nlohmann::json error;
            error["code"] = "INVALID_PARAM";
            error["message"] = "Missing from or to parameter";
            res.body() = error.dump();
            res.prepare_payload();
            return res;
        }

        // 获取趋势统计
        std::vector<accounting::TrendData> trendData = summaryService->getTrendSummary(from, to);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["from"] = from;
        responseBody["to"] = to;
        responseBody["trend_data"] = nlohmann::json::array();

        for (const auto& data : trendData) {
            nlohmann::json dataJson;
            dataJson["month"] = data.month;
            dataJson["total_income"] = data.total_income;
            dataJson["total_expense"] = data.total_expense;
            responseBody["trend_data"].push_back(dataJson);
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
