#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "services/SummaryService.h"

using namespace boost::beast;
using namespace boost::beast::http;
using json = nlohmann::json;

class SummaryController {
public:
    SummaryController();
    ~SummaryController();

    response<string_body> getMonthlySummary(request<string_body>& req);
    response<string_body> getTrendSummary(request<string_body>& req);

private:
    accounting::SummaryService* summaryService;
};
