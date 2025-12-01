#include "service/ClientService.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

using namespace Utils;

ClientService::ClientService(ClientRepository& client_repo) : client_repo_(client_repo) {
}

ClientService::~ClientService() {
}

bool ClientService::createClient(const std::string& name, const std::string& contact_email, uint32_t daily_quota, uint32_t per_minute_quota, Client& created_client) {
    // 验证输入
    if (name.empty() || contact_email.empty()) {
        LOG_ERROR("Client name and contact email cannot be empty");
        return false;
    }

    if (daily_quota == 0 || per_minute_quota == 0) {
        LOG_ERROR("Client quotas cannot be zero");
        return false;
    }

    // 创建客户端对象
    Client client;
    client.name = name;
    client.contact_email = contact_email;
    client.daily_quota = daily_quota;
    client.per_minute_quota = per_minute_quota;
    client.is_enabled = true;
    client.created_at = getCurrentTimeStr();
    client.updated_at = client.created_at;

    // 插入数据库
    if (!client_repo_.insert(client)) {
        LOG_ERROR("Failed to insert client into database");
        return false;
    }

    created_client = client;
    LOG_INFO("Client created successfully: " + std::to_string(client.client_id));
    return true;
}

bool ClientService::updateClient(int64_t client_id, const std::string& name, const std::string& contact_email, uint32_t daily_quota, uint32_t per_minute_quota) {
    // 验证输入
    if (name.empty() || contact_email.empty()) {
        LOG_ERROR("Client name and contact email cannot be empty");
        return false;
    }

    if (daily_quota == 0 || per_minute_quota == 0) {
        LOG_ERROR("Client quotas cannot be zero");
        return false;
    }

    // 获取现有客户端
    Client client;
    if (!client_repo_.getById(client_id, client)) {
        LOG_ERROR("Client not found: " + std::to_string(client_id));
        return false;
    }

    // 更新客户端信息
    client.name = name;
    client.contact_email = contact_email;
    client.daily_quota = daily_quota;
    client.per_minute_quota = per_minute_quota;
    client.updated_at = getCurrentTimeStr();

    // 更新数据库
    if (!client_repo_.update(client)) {
        LOG_ERROR("Failed to update client in database: " + std::to_string(client_id));
        return false;
    }

    LOG_INFO("Client updated successfully: " + std::to_string(client_id));
    return true;
}

bool ClientService::deleteClient(int64_t client_id) {
    // 获取现有客户端
    Client client;
    if (!client_repo_.getById(client_id, client)) {
        LOG_ERROR("Client not found: " + std::to_string(client_id));
        return false;
    }

    // 逻辑删除客户端
    if (!client_repo_.remove(client_id)) {
        LOG_ERROR("Failed to delete client from database: " + std::to_string(client_id));
        return false;
    }

    LOG_INFO("Client deleted successfully: " + std::to_string(client_id));
    return true;
}

bool ClientService::getClientById(int64_t client_id, Client& client) {
    if (!client_repo_.getById(client_id, client)) {
        LOG_ERROR("Client not found: " + std::to_string(client_id));
        return false;
    }

    return true;
}

bool ClientService::getAllClients(std::vector<Client>& clients) {
    if (!client_repo_.getAll(clients)) {
        LOG_ERROR("Failed to get all clients");
        return false;
    }

    return true;
}
