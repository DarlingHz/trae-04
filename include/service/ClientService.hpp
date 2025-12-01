#ifndef CLIENT_SERVICE_HPP
#define CLIENT_SERVICE_HPP

#include <string>
#include <cstdint>
#include <vector>
#include "storage/ClientRepository.hpp"

class ClientService {
public:
    ClientService(ClientRepository& client_repo);
    ~ClientService();

    bool createClient(const std::string& name, const std::string& contact_email, uint32_t daily_quota, uint32_t per_minute_quota, Client& created_client);
    bool updateClient(int64_t client_id, const std::string& name, const std::string& contact_email, uint32_t daily_quota, uint32_t per_minute_quota);
    bool deleteClient(int64_t client_id);
    bool getClientById(int64_t client_id, Client& client);
    bool getAllClients(std::vector<Client>& clients);

private:
    ClientRepository& client_repo_;
};

#endif // CLIENT_SERVICE_HPP
