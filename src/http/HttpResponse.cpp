#include "http/HttpResponse.hpp"
#include <sstream>

void HttpResponse::setStatusCode(StatusCode status_code) {
    status_code_ = status_code;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

void HttpResponse::setBody(const std::string& body) {
    body_ = body;
    setHeader("Content-Length", std::to_string(body_.size()));
}

std::string HttpResponse::toString() const {
    std::ostringstream response_stream;

    // 写入状态行
    response_stream << "HTTP/1.1 " << static_cast<int>(status_code_) << " " << statusCodeToString(status_code_) << "\r\n";

    // 写入响应头
    for (const auto& pair : headers_) {
        response_stream << pair.first << ": " << pair.second << "\r\n";
    }

    // 写入空行分隔头部和体
    response_stream << "\r\n";

    // 写入响应体
    response_stream << body_;

    return response_stream.str();
}

std::string HttpResponse::statusCodeToString(StatusCode status_code) const {
    switch (status_code) {
        case OK:
            return "OK";
        case CREATED:
            return "Created";
        case BAD_REQUEST:
            return "Bad Request";
        case UNAUTHORIZED:
            return "Unauthorized";
        case FORBIDDEN:
            return "Forbidden";
        case NOT_FOUND:
            return "Not Found";
        case METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        case SERVICE_UNAVAILABLE:
            return "Service Unavailable";
        default:
            return "Unknown Status";
    }
}
