#include "http/HttpRequest.hpp"
#include "utils/Utils.hpp"
#include <sstream>
#include <algorithm>

using namespace Utils;

bool HttpRequest::parse(const std::string& request_str) {
    std::istringstream request_stream(request_str);
    std::string line;

    // 解析请求行
    if (!std::getline(request_stream, line)) {
        return false;
    }
    line = trim(line);
    if (!parseRequestLine(line)) {
        return false;
    }

    // 解析请求头
    while (std::getline(request_stream, line)) {
        line = trim(line);
        if (line.empty()) {
            break; // 头部结束
        }
        if (!parseHeaderLine(line)) {
            return false;
        }
    }

    // 解析请求体
    if (headers_.count("Content-Length")) {
        size_t content_length = std::stoul(headers_["Content-Length"]);
        body_.resize(content_length);
        request_stream.read(&body_[0], content_length);
    }

    return true;
}

std::string HttpRequest::getHeader(const std::string& name) const {
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

    for (const auto& pair : headers_) {
        std::string lower_header_name = pair.first;
        std::transform(lower_header_name.begin(), lower_header_name.end(), lower_header_name.begin(), ::tolower);
        if (lower_header_name == lower_name) {
            return pair.second;
        }
    }

    return "";
}

std::string HttpRequest::getQueryParam(const std::string& name) const {
    auto it = query_params_.find(name);
    if (it != query_params_.end()) {
        return it->second;
    }
    return "";
}

bool HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream line_stream(line);
    std::string uri_with_query;

    if (!(line_stream >> method_ >> uri_with_query >> http_version_)) {
        return false;
    }

    // 解析URI和查询参数
    size_t query_start = uri_with_query.find('?');
    if (query_start != std::string::npos) {
        uri_ = uri_with_query.substr(0, query_start);

        std::string query_str = uri_with_query.substr(query_start + 1);
        std::vector<std::string> query_parts = split(query_str, '&');

        for (const std::string& part : query_parts) {
            size_t equals_pos = part.find('=');
            if (equals_pos != std::string::npos) {
                std::string key = part.substr(0, equals_pos);
                std::string value = part.substr(equals_pos + 1);
                query_params_[key] = value;
            } else {
                query_params_[part] = "";
            }
        }
    } else {
        uri_ = uri_with_query;
    }

    return true;
}

bool HttpRequest::parseHeaderLine(const std::string& line) {
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }

    std::string name = trim(line.substr(0, colon_pos));
    std::string value = trim(line.substr(colon_pos + 1));

    headers_[name] = value;
    return true;
}
