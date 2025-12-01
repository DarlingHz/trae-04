#include "App.hpp"
#include "utils/Logger.hpp"
#include "nlohmann/json.hpp"
#include <iostream>

using json = nlohmann::json;

App::App(const std::string& config_file_path) 
    : config_file_path_(config_file_path), 
      config_(nullptr), 
      db_(nullptr), 
      client_repo_(nullptr), 
      api_key_repo_(nullptr), 
      call_log_repo_(nullptr), 
      client_service_(nullptr), 
      api_key_service_(nullptr), 
      quota_service_(nullptr), 
      stats_service_(nullptr), 
      http_server_(nullptr) {
}

App::~App() {
    stop();
    cleanup();
}

bool App::init() {
    // 初始化日志
    Logger::getInstance().init("api_quota_server.log");
    Logger::getInstance().setLevel(Logger::Level::INFO);

    LOG_INFO("Initializing API Quota Management Service...");

    // 加载配置
    config_ = std::make_unique<Config>();
    if (!config_->load(config_file_path_)) {
        LOG_ERROR("Failed to load configuration");
        return false;
    }

    // 初始化数据库
    db_ = std::make_unique<Database>();
    if (!db_->open(config_->getDbPath())) {
        LOG_ERROR("Failed to open database");
        return false;
    }

    // 初始化数据访问层
    client_repo_ = std::make_unique<ClientRepository>(*db_.get());
    api_key_repo_ = std::make_unique<ApiKeyRepository>(*db_.get());
    call_log_repo_ = std::make_unique<CallLogRepository>(*db_.get());

    // 创建数据库表
    if (!client_repo_->createTable() || 
        !api_key_repo_->createTable() || 
        !call_log_repo_->createTable()) {
        LOG_ERROR("Failed to create database tables");
        return false;
    }

    // 初始化业务逻辑层
    client_service_ = std::make_unique<ClientService>(*client_repo_.get());
    api_key_service_ = std::make_unique<ApiKeyService>(*api_key_repo_.get(), *client_repo_.get());
    quota_service_ = std::make_unique<QuotaService>(*client_repo_.get(), *api_key_repo_.get(), *call_log_repo_.get());
    stats_service_ = std::make_unique<StatsService>(*call_log_repo_.get(), *client_repo_.get());

    // 初始化HTTP服务器
    uint16_t port = config_->getPort();
    uint32_t thread_pool_size = config_->getThreadPoolSize();
    http_server_ = std::make_unique<HttpServer>(port, thread_pool_size);

    // 注册HTTP处理程序
    registerHttpHandlers();

    LOG_INFO("API Quota Management Service initialized successfully");
    return true;
}

bool App::run() {
    LOG_INFO("Starting API Quota Management Service...");

    // 启动HTTP服务器
    if (!http_server_->start()) {
        LOG_ERROR("Failed to start HTTP server");
        return false;
    }

    LOG_INFO("API Quota Management Service is running");

    // 主线程等待用户输入停止服务
    std::string input;
    while (true) {
        std::cout << "Enter 'quit' to stop the service: ";
        std::getline(std::cin, input);
        if (input == "quit") {
            break;
        }
    }

    return true;
}

void App::stop() {
    LOG_INFO("Stopping API Quota Management Service...");

    // 停止HTTP服务器
    if (http_server_) {
        http_server_->stop();
    }

    // 关闭数据库
    if (db_) {
        db_->close();
    }

    LOG_INFO("API Quota Management Service stopped successfully");
}

void App::cleanup() {
    // 释放资源
    http_server_.reset();
    stats_service_.reset();
    quota_service_.reset();
    api_key_service_.reset();
    client_service_.reset();
    call_log_repo_.reset();
    api_key_repo_.reset();
    client_repo_.reset();
    db_.reset();
    config_.reset();
}

void App::registerHttpHandlers() {
    // 客户端管理接口
    http_server_->registerHandler("POST", "/clients", [this](const HttpRequest& request, HttpResponse& response) {
        handleCreateClient(request, response);
    });

    http_server_->registerHandler("GET", "/clients", [this](const HttpRequest& request, HttpResponse& response) {
        handleGetAllClients(request, response);
    });

    http_server_->registerHandler("GET", "/clients/{client_id}", [this](const HttpRequest& request, HttpResponse& response) {
        handleGetClient(request, response);
    });

    http_server_->registerHandler("PUT", "/clients/{client_id}", [this](const HttpRequest& request, HttpResponse& response) {
        handleUpdateClient(request, response);
    });

    http_server_->registerHandler("DELETE", "/clients/{client_id}", [this](const HttpRequest& request, HttpResponse& response) {
        handleDeleteClient(request, response);
    });

    // API Key管理接口
    http_server_->registerHandler("POST", "/clients/{client_id}/keys", [this](const HttpRequest& request, HttpResponse& response) {
        handleCreateApiKey(request, response);
    });

    http_server_->registerHandler("GET", "/clients/{client_id}/keys", [this](const HttpRequest& request, HttpResponse& response) {
        handleGetApiKeys(request, response);
    });

    http_server_->registerHandler("POST", "/keys/{key_id}/revoke", [this](const HttpRequest& request, HttpResponse& response) {
        handleRevokeApiKey(request, response);
    });

    // 配额校验接口
    http_server_->registerHandler("POST", "/quota/check", [this](const HttpRequest& request, HttpResponse& response) {
        handleQuotaCheck(request, response);
    });

    // 统计接口
    http_server_->registerHandler("GET", "/stats/clients/top", [this](const HttpRequest& request, HttpResponse& response) {
        handleGetTopClients(request, response);
    });

    http_server_->registerHandler("GET", "/stats/clients/{client_id}/summary", [this](const HttpRequest& request, HttpResponse& response) {
        handleGetClientStats(request, response);
    });
}

// 客户端管理接口处理函数
void App::handleCreateClient(const HttpRequest& request, HttpResponse& response) {
    try {
        json request_body = json::parse(request.getBody());

        // 验证请求体
        if (!request_body.contains("name") || 
            !request_body.contains("contact_email") || 
            !request_body.contains("daily_quota") || 
            !request_body.contains("per_minute_quota")) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Missing required fields"}}.dump());
            return;
        }

        // 创建客户端
        Client client;
        std::string name = request_body["name"].get<std::string>();
        std::string contact_email = request_body["contact_email"].get<std::string>();
        uint32_t daily_quota = request_body["daily_quota"].get<uint32_t>();
        uint32_t per_minute_quota = request_body["per_minute_quota"].get<uint32_t>();

        if (!client_service_->createClient(name, contact_email, daily_quota, per_minute_quota, client)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to create client"}}.dump());
            return;
        }

        // 构造响应
        json response_body = {
            {"client_id", client.client_id},
            {"name", client.name},
            {"contact_email", client.contact_email},
            {"daily_quota", client.daily_quota},
            {"per_minute_quota", client.per_minute_quota},
            {"created_at", client.created_at},
            {"updated_at", client.updated_at}
        };

        response.setStatusCode(HttpResponse::CREATED);
        response.setBody(response_body.dump());
    } catch (const json::parse_error& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid JSON format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

void App::handleGetAllClients(const HttpRequest& request, HttpResponse& response) {
    try {
        std::vector<Client> clients;
        if (!client_service_->getAllClients(clients)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to get all clients"}}.dump());
            return;
        }

        // 构造响应
        json response_body = json::array();
        for (const Client& client : clients) {
            response_body.push_back({
                {"client_id", client.client_id},
                {"name", client.name},
                {"contact_email", client.contact_email},
                {"daily_quota", client.daily_quota},
                {"per_minute_quota", client.per_minute_quota},
                {"created_at", client.created_at},
                {"updated_at", client.updated_at}
            });
        }

        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

void App::handleGetClient(const HttpRequest& request, HttpResponse& response) {
    try {
        // 从URI中提取client_id
        std::string uri = request.getUri();
        size_t last_slash = uri.find_last_of('/');
        if (last_slash == std::string::npos) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Invalid client ID"}}.dump());
            return;
        }

        std::string client_id_str = uri.substr(last_slash + 1);
        uint64_t client_id = std::stoull(client_id_str);

        // 获取客户端
        Client client;
        if (!client_service_->getClientById(client_id, client)) {
            response.setStatusCode(HttpResponse::NOT_FOUND);
            response.setBody(json{{"error", "Client not found"}}.dump());
            return;
        }

        // 构造响应
        json response_body = {
            {"client_id", client.client_id},
            {"name", client.name},
            {"contact_email", client.contact_email},
            {"daily_quota", client.daily_quota},
            {"per_minute_quota", client.per_minute_quota},
            {"created_at", client.created_at},
            {"updated_at", client.updated_at}
        };

        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid client ID format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

void App::handleUpdateClient(const HttpRequest& request, HttpResponse& response) {
    try {
        // 从URI中提取client_id
        std::string uri = request.getUri();
        size_t last_slash = uri.find_last_of('/');
        if (last_slash == std::string::npos) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Invalid client ID"}}.dump());
            return;
        }

        std::string client_id_str = uri.substr(last_slash + 1);
        uint64_t client_id = std::stoull(client_id_str);

        // 解析请求体
        json request_body = json::parse(request.getBody());

        // 获取现有客户端
        Client client;
        if (!client_service_->getClientById(client_id, client)) {
            response.setStatusCode(HttpResponse::NOT_FOUND);
            response.setBody(json{{"error", "Client not found"}}.dump());
            return;
        }

        // 更新客户端信息
        if (request_body.contains("name")) {
            client.name = request_body["name"].get<std::string>();
        }
        if (request_body.contains("contact_email")) {
            client.contact_email = request_body["contact_email"].get<std::string>();
        }
        if (request_body.contains("daily_quota")) {
            client.daily_quota = request_body["daily_quota"].get<uint32_t>();
        }
        if (request_body.contains("per_minute_quota")) {
            client.per_minute_quota = request_body["per_minute_quota"].get<uint32_t>();
        }

        // 保存更新后的客户端
        if (!client_service_->updateClient(client.client_id, client.name, client.contact_email, client.daily_quota, client.per_minute_quota)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to update client"}}.dump());
            return;
        }

        // 构造响应
        json response_body = {
            {"client_id", client.client_id},
            {"name", client.name},
            {"contact_email", client.contact_email},
            {"daily_quota", client.daily_quota},
            {"per_minute_quota", client.per_minute_quota},
            {"created_at", client.created_at},
            {"updated_at", client.updated_at}
        };

        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const json::parse_error& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid JSON format"}}.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid client ID format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

void App::handleDeleteClient(const HttpRequest& request, HttpResponse& response) {
    try {
        // 从URI中提取client_id
        std::string uri = request.getUri();
        size_t last_slash = uri.find_last_of('/');
        if (last_slash == std::string::npos) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Invalid client ID"}}.dump());
            return;
        }

        std::string client_id_str = uri.substr(last_slash + 1);
        uint64_t client_id = std::stoull(client_id_str);

        // 删除客户端
        if (!client_service_->deleteClient(client_id)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to delete client"}}.dump());
            return;
        }

        // 构造响应
        json response_body = {{"message", "Client deleted successfully"}};
        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid client ID format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

// API Key管理接口处理函数
void App::handleCreateApiKey(const HttpRequest& request, HttpResponse& response) {
    try {
        // 从URI中提取client_id
        std::string uri = request.getUri();
        size_t last_slash = uri.find_last_of('/');
        size_t second_last_slash = uri.find_last_of('/', last_slash - 1);
        if (second_last_slash == std::string::npos) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Invalid client ID"}}.dump());
            return;
        }

        std::string client_id_str = uri.substr(second_last_slash + 1, last_slash - second_last_slash - 1);
        uint64_t client_id = std::stoull(client_id_str);

        // 解析请求体（可选的过期时间）
        json request_body;
        if (!request.getBody().empty()) {
            request_body = json::parse(request.getBody());
        }

        std::string expired_at; 
        if (request_body.contains("expired_at")) {
            expired_at = request_body["expired_at"].get<std::string>();
        }

        // 创建API Key
        ApiKey api_key;
        if (!api_key_service_->createApiKey(client_id, expired_at, api_key)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to create API key"}}.dump());
            return;
        }

        // 构造响应
        json response_body = {
            {"key_id", api_key.key_id},
            {"client_id", api_key.client_id},
            {"api_key", api_key.api_key},
            {"is_revoked", api_key.is_revoked},
            {"expired_at", api_key.expired_at},
            {"created_at", api_key.created_at}
        };

        response.setStatusCode(HttpResponse::CREATED);
        response.setBody(response_body.dump());
    } catch (const json::parse_error& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid JSON format"}}.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid client ID format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

void App::handleGetApiKeys(const HttpRequest& request, HttpResponse& response) {
    try {
        // 从URI中提取client_id
        std::string uri = request.getUri();
        size_t last_slash = uri.find_last_of('/');
        size_t second_last_slash = uri.find_last_of('/', last_slash - 1);
        if (second_last_slash == std::string::npos) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Invalid client ID"}}.dump());
            return;
        }

        std::string client_id_str = uri.substr(second_last_slash + 1, last_slash - second_last_slash - 1);
        uint64_t client_id = std::stoull(client_id_str);

        // 获取客户端的所有API Key
        std::vector<ApiKey> api_keys;
        if (!api_key_service_->getApiKeysByClientId(client_id, api_keys)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to get API keys for client"}}.dump());
            return;
        }

        // 构造响应
        json response_body = json::array();
        for (const ApiKey& api_key : api_keys) {
            response_body.push_back({
                {"key_id", api_key.key_id},
                {"client_id", api_key.client_id},
                {"api_key", api_key.api_key},
                {"is_revoked", api_key.is_revoked},
                {"expired_at", api_key.expired_at},
                {"created_at", api_key.created_at}
            });
        }

        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid client ID format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

void App::handleRevokeApiKey(const HttpRequest& request, HttpResponse& response) {
    try {
        // 从URI中提取key_id
        std::string uri = request.getUri();
        size_t last_slash = uri.find_last_of('/');
        if (last_slash == std::string::npos) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Invalid key ID"}}.dump());
            return;
        }

        std::string key_id_str = uri.substr(last_slash + 1);
        uint64_t key_id = std::stoull(key_id_str);

        // 吊销API Key
        if (!api_key_service_->revokeApiKey(key_id)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to revoke API key"}}.dump());
            return;
        }

        // 构造响应
        json response_body = {{"message", "API key revoked successfully"}};
        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid key ID format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

// 配额校验接口处理函数
void App::handleQuotaCheck(const HttpRequest& request, HttpResponse& response) {
    try {
        // 解析请求体
        json request_body = json::parse(request.getBody());

        // 验证请求体
        if (!request_body.contains("api_key") || 
            !request_body.contains("endpoint") || 
            !request_body.contains("weight")) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Missing required fields"}}.dump());
            return;
        }

        // 提取请求参数
        std::string api_key = request_body["api_key"].get<std::string>();
        std::string endpoint = request_body["endpoint"].get<std::string>();
        uint32_t weight = request_body["weight"].get<uint32_t>();

        // 检查配额
        QuotaCheckResult result;
        if (!quota_service_->checkQuota(api_key, endpoint, weight, result)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to check quota"}}.dump());
            return;
        }
        if (!result.allowed) {
            response.setStatusCode(HttpResponse::FORBIDDEN);
            response.setBody(json{{"error", result.reason}, {"retry_after_seconds", result.retry_after_seconds}}.dump());
            return;
        }

        // 构造响应
        json response_body = {
            {"allowed", result.allowed},
            {"reason", result.reason},
            {"remaining_in_minute", result.remaining_in_minute},
            {"remaining_in_day", result.remaining_in_day}
        };

        response.setStatusCode(HttpResponse::OK);

        response.setBody(response_body.dump());
    } catch (const json::parse_error& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid JSON format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

// 统计接口处理函数
void App::handleGetTopClients(const HttpRequest& request, HttpResponse& response) {
    try {
        // 获取查询参数
        std::string by = request.getQueryParam("by");
        std::string limit_str = request.getQueryParam("limit");

        // 默认值
        if (by.empty()) {
            by = "daily_calls";
        }
        uint32_t limit = 10;
        if (!limit_str.empty()) {
            limit = std::stoul(limit_str);
        }

        // 获取每日调用量最高的客户端
        std::string date = "";
        std::vector<std::pair<int64_t, uint32_t>> top_clients;
        if (!stats_service_->getTopClientsByDailyCalls(date, limit, top_clients)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to get top clients"}}.dump());
            return;
        }

        // 构造响应
        json response_body = json::array();
        for (const std::pair<int64_t, uint32_t>& top_client : top_clients) {
            response_body.push_back({
                {"client_id", top_client.first},
                {"call_count", top_client.second}
            });
        }

        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid limit format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}

void App::handleGetClientStats(const HttpRequest& request, HttpResponse& response) {
    try {
        // 从URI中提取client_id
        std::string uri = request.getUri();
        size_t last_slash = uri.find_last_of('/');
        size_t second_last_slash = uri.find_last_of('/', last_slash - 1);
        if (second_last_slash == std::string::npos) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Invalid client ID"}}.dump());
            return;
        }

        std::string client_id_str = uri.substr(second_last_slash + 1, last_slash - second_last_slash - 1);
        uint64_t client_id = std::stoull(client_id_str);

        // 获取查询参数
        std::string from = request.getQueryParam("from");
        std::string to = request.getQueryParam("to");

        // 验证日期参数
        if (from.empty() || to.empty()) {
            response.setStatusCode(HttpResponse::BAD_REQUEST);
            response.setBody(json{{"error", "Missing required date parameters"}}.dump());
            return;
        }

        // 获取客户端统计数据
        ClientStats stats;
        if (!stats_service_->getClientStats(client_id, from, to, stats)) {
            response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
            response.setBody(json{{"error", "Failed to get client stats"}}.dump());
            return;
        }

        // 构造响应
        json response_body = {
            {"client_id", stats.client_id},
            {"client_name", stats.client_name},
            {"total_calls", stats.total_calls},
            {"allowed_calls", stats.allowed_calls},
            {"rejected_calls", stats.rejected_calls},
            {"rejection_reasons", stats.rejection_reasons}
        };

        response.setStatusCode(HttpResponse::OK);
        response.setBody(response_body.dump());
    } catch (const std::invalid_argument& e) {
        response.setStatusCode(HttpResponse::BAD_REQUEST);
        response.setBody(json{{"error", "Invalid client ID or date format"}}.dump());
    } catch (const std::exception& e) {
        response.setStatusCode(HttpResponse::INTERNAL_SERVER_ERROR);
        response.setBody(json{{"error", e.what()}}.dump());
    }
}
