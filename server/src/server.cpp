#include "server.hpp"
#include "client_handler.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <stdexcept>
#include <cstring>
#include <ostream>
#include <iostream>

Server::Server(int port) : port_(port), server_fd_(-1), running_(false) {}

Server::~Server()
{
    stop();
}

void Server::setup_server()
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
    {
        throw std::runtime_error("Falha ao criar socket");
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(server_fd_);
        throw std::runtime_error("Falha ao configurar socket");
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        close(server_fd_);
        throw std::runtime_error("Falha ao fazer bind");
    }

    if (listen(server_fd_, 5) < 0)
    {
        close(server_fd_);
        throw std::runtime_error("Falha ao ouvir conexões");
    }
}

void Server::accept_connections()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (running_)
    {
        int client_fd = accept(server_fd_, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            if (running_)
            {
                std::cerr << "Erro ao aceitar conexão" << std::endl;
            }
            continue;
        }

        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.emplace_back(ClientInfo{client_fd, "", ""});

        std::thread client_thread(&ClientHandler::handle_client,
                                  client_fd,
                                  std::ref(clients_),
                                  std::ref(clients_mutex_));
        client_thread.detach();
    }
}

void Server::start()
{
    if (running_)
        return;

    setup_server();
    running_ = true;

    std::cout << "Servidor iniciado na porta " << port_ << std::endl;
    accept_connections();
}

void Server::stop()
{
    if (!running_)
        return;

    running_ = false;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto &client : clients_)
        {
            close(client.fd);
        }
        clients_.clear();
    }

    if (server_fd_ >= 0)
    {
        close(server_fd_);
        server_fd_ = -1;
    }

    std::cout << "Servidor encerrado" << std::endl;
}