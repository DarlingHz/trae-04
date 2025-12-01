#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "services/CategoryService.h"

using namespace boost::beast;
using namespace boost::beast::http;
using json = nlohmann::json;

class CategoryController {
public:
    CategoryController();
    ~CategoryController();

    response<string_body> createCategory(request<string_body>& req);
    response<string_body> getCategories(request<string_body>& req);
    response<string_body> updateCategory(request<string_body>& req, int id);
    response<string_body> deleteCategory(int id);

private:
    accounting::CategoryService* categoryService;
};
