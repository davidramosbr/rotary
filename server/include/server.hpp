#pragma once

#include "common.hpp"
#include <atomic>
#include <mutex>

class Server
{
public:
    Server(int port);
    ~Server();

    void start();
    void stop();

private:
    void setup_server();
    void accept_connections();

    int port_;
    int server_fd_;
    std::atomic<bool> running_;
    std::vector<ClientInfo> clients_;
    std::mutex clients_mutex_;
};