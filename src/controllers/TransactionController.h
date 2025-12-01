#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "services/TransactionService.h"

using namespace boost::beast;
using namespace boost::beast::http;
using json = nlohmann::json;

class TransactionController {
public:
    TransactionController();
    ~TransactionController();

    response<string_body> createTransaction(request<string_body>& req);
    response<string_body> getTransactions(request<string_body>& req);
    response<string_body> updateTransaction(request<string_body>& req, int id);
    response<string_body> deleteTransaction(request<string_body>& req, int id);

private:
    accounting::TransactionService* transactionService;
};
