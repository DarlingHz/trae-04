#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

class HttpResponse {
public:
    enum StatusCode {
        OK = 200,
        CREATED = 201,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        INTERNAL_SERVER_ERROR = 500,
        SERVICE_UNAVAILABLE = 503
    };

    HttpResponse() : status_code_(OK) {};
    ~HttpResponse() = default;

    void setStatusCode(StatusCode status_code);
    void setHeader(const std::string& name, const std::string& value);
    void setBody(const std::string& body);

    std::string toString() const;

private:
    std::string statusCodeToString(StatusCode status_code) const;

    StatusCode status_code_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};

#endif // HTTP_RESPONSE_HPP
