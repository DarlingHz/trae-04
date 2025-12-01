#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    HttpRequest() = default;
    ~HttpRequest() = default;

    bool parse(const std::string& request_str);

    const std::string& getMethod() const { return method_; }
    const std::string& getUri() const { return uri_; }
    const std::string& getHttpVersion() const { return http_version_; }
    const std::map<std::string, std::string>& getHeaders() const { return headers_; }
    const std::string& getBody() const { return body_; }

    std::string getHeader(const std::string& name) const;
    std::string getQueryParam(const std::string& name) const;

private:
    bool parseRequestLine(const std::string& line);
    bool parseHeaderLine(const std::string& line);

    std::string method_;
    std::string uri_;
    std::string http_version_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> query_params_;
    std::string body_;
};

#endif // HTTP_REQUEST_HPP
