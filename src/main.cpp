#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "controllers/AccountController.h"
#include "controllers/CategoryController.h"
#include "controllers/TransactionController.h"
#include "controllers/BudgetController.h"
#include "controllers/SummaryController.h"
#include "dao/Database.h"

using namespace boost::asio;
using namespace boost::beast;
using namespace boost::beast::http;
using json = nlohmann::json;

// 处理HTTP请求的函数
template <typename Body, typename Allocator>
response<string_body> handle_request(request<Body, basic_fields<Allocator>>&& req,
                                        AccountController& accountController,
                                        CategoryController& categoryController,
                                        TransactionController& transactionController,
                                        BudgetController& budgetController,
                                        SummaryController& summaryController)
{
    // 解析请求路径和方法
    std::string path = std::string(req.target());
    std::string method = std::string(req.method_string());

    // 处理不同的请求
    if (method == "POST" && path == "/accounts") {
        return accountController.createAccount(req);
    } else if (method == "GET" && path == "/accounts") {
        return accountController.getAccounts(req);
    } else if (method == "PUT" && path.substr(0, 10) == "/accounts/") {
        int id = std::stoi(path.substr(10));
        return accountController.updateAccount(req, id);
    } else if (method == "DELETE" && path.substr(0, 10) == "/accounts/") {
        int id = std::stoi(path.substr(10));
        return accountController.deleteAccount(id);
    } else if (method == "POST" && path == "/categories") {
        return categoryController.createCategory(req);
    } else if (method == "GET" && path == "/categories") {
        return categoryController.getCategories(req);
    } else if (method == "PUT" && path.substr(0, 13) == "/categories/") {
        int id = std::stoi(path.substr(13));
        return categoryController.updateCategory(req, id);
    } else if (method == "DELETE" && path.substr(0, 13) == "/categories/") {
        int id = std::stoi(path.substr(13));
        return categoryController.deleteCategory(id);
    } else if (method == "POST" && path == "/transactions") {
        return transactionController.createTransaction(req);
    } else if (method == "GET" && path == "/transactions") {
        return transactionController.getTransactions(req);
    } else if (method == "PUT" && path.substr(0, 15) == "/transactions/") {
        int id = std::stoi(path.substr(15));
        return transactionController.updateTransaction(req, id);
    } else if (method == "DELETE" && path.substr(0, 15) == "/transactions/") {
        int id = std::stoi(path.substr(15));
        return transactionController.deleteTransaction(req, id);
    } else if (method == "PUT" && path == "/budgets") {
        return budgetController.setBudget(req);
    } else if (method == "GET" && path == "/budgets") {
        return budgetController.getBudgets(req);
    } else if (method == "GET" && path.substr(0, 15) == "/summary/monthly") {
        return summaryController.getMonthlySummary(req);
    } else if (method == "GET" && path.substr(0, 14) == "/summary/trend") {
        return summaryController.getTrendSummary(req);
    }

    // 如果没有匹配的路由，返回404
    response<string_body> res(status::not_found, req.version());
    res.set(field::server, "Accounting Server");
    res.set(field::content_type, "application/json");
    json error;
    error["code"] = "NOT_FOUND";
    error["message"] = "Route not found";
    res.body() = error.dump();
    res.prepare_payload();
    return res;
}

int main(int argc, char* argv[]) {
    try {
        // 初始化数据库
        accounting::Database& db = accounting::Database::getInstance();
        if (!db.open("accounting.db")) {
            std::cerr << "Failed to open database" << std::endl;
            return 1;
        }

        // 创建控制器实例
        AccountController accountController;
        CategoryController categoryController;
        TransactionController transactionController;
        BudgetController budgetController;
        SummaryController summaryController;

        // 设置服务器端口
        int port = 8080;
        if (argc > 1) {
            port = std::stoi(argv[1]);
        }

        // 初始化Boost.Asio
        io_context io_context;
        ip::tcp::acceptor acceptor(io_context, ip::tcp::endpoint(ip::tcp::v4(), port));

        std::cout << "Server started on port " << port << std::endl;

        // 接受连接并处理请求
        while (true) {
            ip::tcp::socket socket(io_context);
            acceptor.accept(socket);

            // 使用线程处理每个连接
            std::thread([socket = std::move(socket), 
                          &accountController, &categoryController, 
                          &transactionController, &budgetController, 
                          &summaryController]() mutable {
                try {
                    // 读取请求
                    flat_buffer buffer;
                    request<string_body> req;
                    read(socket, buffer, req);

                    // 处理请求
                    auto res = handle_request(std::move(req), 
                                               accountController, categoryController, 
                                               transactionController, budgetController, 
                                               summaryController);

                    // 发送响应
                    write(socket, res);
                } catch (const std::exception& e) {
                    std::cerr << "Error handling request: " << e.what() << std::endl;
                }
            }).detach();
        }

        // 关闭数据库
        db.close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
