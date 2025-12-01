#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <thread>
#include <functional>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class HttpServer {
public:
    using RequestHandler = std::function<void(const HttpRequest&, HttpResponse&)>;

    HttpServer(uint16_t port, uint32_t thread_pool_size = 10);
    ~HttpServer();

    bool start();
    void stop();

    void registerHandler(const std::string& method, const std::string& path, RequestHandler handler);

private:
    struct HandlerKey {
        std::string method;
        std::string path;

        HandlerKey(const std::string& m, const std::string& p) : method(m), path(p) {};

        bool operator<(const HandlerKey& other) const {
            if (method != other.method) {
                return method < other.method;
            }
            return path < other.path;
        }
    };

    void acceptConnections();
    void handleClient(int client_socket);
    void processRequest(const HttpRequest& request, HttpResponse& response);

    uint16_t port_;
    uint32_t thread_pool_size_;
    int server_socket_;
    bool is_running_;
    std::vector<std::thread> thread_pool_;
    std::map<HandlerKey, RequestHandler> handlers_;
};

#endif // HTTP_SERVER_HPP
