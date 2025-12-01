#ifndef APP_HPP
#define APP_HPP

#include <string>
#include <memory>
#include "config/Config.hpp"
#include "utils/Logger.hpp"
#include "storage/Database.hpp"
#include "storage/ClientRepository.hpp"
#include "storage/ApiKeyRepository.hpp"
#include "storage/CallLogRepository.hpp"
#include "service/ClientService.hpp"
#include "service/ApiKeyService.hpp"
#include "service/QuotaService.hpp"
#include "service/StatsService.hpp"
#include "http/HttpServer.hpp"

class App {
public:
    App(const std::string& config_file_path);
    ~App();

    bool init();
    bool run();
    void stop();
    void cleanup();

private:
    void registerHttpHandlers();

    // 客户端管理接口处理函数
    void handleCreateClient(const HttpRequest& request, HttpResponse& response);
    void handleGetAllClients(const HttpRequest& request, HttpResponse& response);
    void handleGetClient(const HttpRequest& request, HttpResponse& response);
    void handleUpdateClient(const HttpRequest& request, HttpResponse& response);
    void handleDeleteClient(const HttpRequest& request, HttpResponse& response);

    // API Key管理接口处理函数
    void handleCreateApiKey(const HttpRequest& request, HttpResponse& response);
    void handleGetApiKeys(const HttpRequest& request, HttpResponse& response);
    void handleRevokeApiKey(const HttpRequest& request, HttpResponse& response);

    // 配额校验接口处理函数
    void handleQuotaCheck(const HttpRequest& request, HttpResponse& response);

    // 统计接口处理函数
    void handleGetTopClients(const HttpRequest& request, HttpResponse& response);
    void handleGetClientStats(const HttpRequest& request, HttpResponse& response);

    // 配置
    std::unique_ptr<Config> config_;

    // 数据库
    std::unique_ptr<Database> db_;

    // 数据访问层
    std::unique_ptr<ClientRepository> client_repo_;
    std::unique_ptr<ApiKeyRepository> api_key_repo_;
    std::unique_ptr<CallLogRepository> call_log_repo_;

    // 业务逻辑层
    std::unique_ptr<ClientService> client_service_;
    std::unique_ptr<ApiKeyService> api_key_service_;
    std::unique_ptr<QuotaService> quota_service_;
    std::unique_ptr<StatsService> stats_service_;

    // HTTP 服务器
    std::unique_ptr<HttpServer> http_server_;
    std::string config_file_path_;
};

#endif // APP_HPP
