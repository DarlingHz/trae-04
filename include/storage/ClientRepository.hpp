#ifndef CLIENT_REPOSITORY_HPP
#define CLIENT_REPOSITORY_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include "Database.hpp"

struct Client {
    int64_t client_id;
    std::string name;
    std::string contact_email;
    uint32_t daily_quota;
    uint32_t per_minute_quota;
    bool is_enabled;
    std::string created_at;
    std::string updated_at;
};

class ClientRepository {
public:
    ClientRepository(Database& db);
    ~ClientRepository();

    bool createTable();
    bool insert(Client& client);
    bool update(const Client& client);
    bool remove(int64_t client_id);
    bool getById(int64_t client_id, Client& client);
    bool getAll(std::vector<Client>& clients);

private:
    Database& db_;
};

#endif // CLIENT_REPOSITORY_HPP
