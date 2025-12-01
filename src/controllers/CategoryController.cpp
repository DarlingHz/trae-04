#include "CategoryController.h"

CategoryController::CategoryController() {
    categoryService = new accounting::CategoryService();
}

CategoryController::~CategoryController() {
    delete categoryService;
}

response<string_body> CategoryController::createCategory(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 解析请求体
        nlohmann::json requestBody = nlohmann::json::parse(req.body());

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

        // 创建分类
        accounting::Category category;
        bool success = categoryService->createCategory(name, type, category);

        if (success) {
            // 构建响应
            nlohmann::json responseBody;
            responseBody["id"] = category.getId();
            responseBody["name"] = category.getName();
            responseBody["type"] = category.getType();
            res.body() = responseBody.dump();
        } else {
            res.result(status::internal_server_error);
            nlohmann::json error;
            error["code"] = "INTERNAL_ERROR";
            error["message"] = "Failed to create category";
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

response<string_body> CategoryController::getCategories(request<string_body>& req) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 获取分类列表
        std::vector<accounting::Category> categories = categoryService->getAllCategories();

        // 构建响应
        nlohmann::json responseBody;
        responseBody["categories"] = nlohmann::json::array();

        for (const auto& category : categories) {
            nlohmann::json categoryJson;
            categoryJson["id"] = category.getId();
            categoryJson["name"] = category.getName();
            categoryJson["type"] = category.getType();
            responseBody["categories"].push_back(categoryJson);
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

response<string_body> CategoryController::updateCategory(request<string_body>& req, int id) {
    response<string_body> res(status::ok, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 解析请求体
        nlohmann::json requestBody = nlohmann::json::parse(req.body());

        // 提取字段值
        std::string name;
        std::string type;

        if (requestBody.contains("name")) {
            name = requestBody["name"];
        }

        if (requestBody.contains("type")) {
            type = requestBody["type"];
        }

        // 更新分类
        bool success = categoryService->updateCategory(id, name, type);

        if (success) {
            // 获取更新后的分类信息
            accounting::Category category;
            categoryService->getCategoryById(id, category);

            // 构建响应
            nlohmann::json responseBody;
            responseBody["id"] = category.getId();
            responseBody["name"] = category.getName();
            responseBody["type"] = category.getType();
            res.body() = responseBody.dump();
        } else {
            res.result(status::internal_server_error);
            nlohmann::json error;
            error["code"] = "INTERNAL_ERROR";
            error["message"] = "Failed to update category";
            res.body() = error.dump();
        }
    } catch (const nlohmann::json::parse_error& e) {
        res.result(status::bad_request);
        nlohmann::json error;
        error["code"] = "INVALID_JSON";
        error["message"] = "Failed to parse JSON";
        res.body() = error.dump();
    } catch (const std::exception& e) {
        if (std::string(e.what()) == "Category not found") {
            res.result(status::not_found);
            nlohmann::json error;
            error["code"] = "NOT_FOUND";
            error["message"] = "Category not found";
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

response<string_body> CategoryController::deleteCategory(int id) {
    response<string_body> res(status::ok, 11);
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");

    try {
        // 删除分类
        categoryService->deleteCategory(id);

        // 构建响应
        nlohmann::json responseBody;
        responseBody["message"] = "Category deleted successfully";
        res.body() = responseBody.dump();
    } catch (const std::exception& e) {
        if (std::string(e.what()) == "Category not found") {
            res.result(status::not_found);
            nlohmann::json error;
            error["code"] = "NOT_FOUND";
            error["message"] = "Category not found";
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
