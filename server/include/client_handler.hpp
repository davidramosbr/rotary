#pragma once

#include "common.hpp"
#include <vector>
#include <mutex>

class ClientHandler
{
public:
    static void handle_client(int client_fd,
                              std::vector<ClientInfo> &clients,
                              std::mutex &clients_mutex);

private:
    static void broadcast_message(const std::string &message,
                                  const std::string &sender_id,
                                  std::vector<ClientInfo> &clients,
                                  std::mutex &clients_mutex);

    static bool is_http_request(const std::string &message);
};