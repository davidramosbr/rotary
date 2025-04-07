#include "client_handler.hpp"
#include "common.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>

void ClientHandler::handle_client(int client_fd,
                                  std::vector<ClientInfo> &clients,
                                  std::mutex &clients_mutex)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_fd, (struct sockaddr *)&addr, &addr_len);
    std::string client_ip = inet_ntoa(addr.sin_addr);
    std::string client_id = generate_client_id();

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = std::find_if(clients.begin(), clients.end(),
                               [client_fd](const ClientInfo &c)
                               { return c.fd == client_fd; });
        if (it != clients.end())
        {
            it->id = client_id;
            it->ip = client_ip;
        }
    }

    std::ostringstream welcome_msg;
    welcome_msg << "Sistema: Conexão estabelecida. Seu ID: " << client_id << "\n";
    std::string welcome_str = welcome_msg.str();
    send(client_fd, welcome_str.c_str(), welcome_str.size(), 0);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);
    struct timeval timeout = {1, 0};

    if (select(client_fd + 1, &readfds, nullptr, nullptr, &timeout) > 0)
    {
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            std::string first_message(buffer);

            if (is_http_request(first_message))
            {
                const std::string response = "HTTP/1.1 403 Forbidden\r\n"
                                             "Connection: close\r\n\r\n";
                send(client_fd, response.c_str(), response.size(), 0);
                close(client_fd);
                return;
            }

            if (!first_message.empty() && first_message != "\n")
            {
                if (first_message.back() == '\n')
                {
                    first_message.pop_back();
                }
                std::string formatted_msg = "[" + client_id + "]: " + first_message + "\n";
                broadcast_message(formatted_msg, client_id, clients, clients_mutex);
            }
        }
    }

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0)
        {
            break;
        }

        buffer[bytes_received] = '\0';
        std::string message(buffer);

        if (message.empty() || message == "\n")
        {
            continue;
        }

        if (message.back() == '\n')
        {
            message.pop_back();
        }

        std::string formatted_msg = "[" + client_id + "]: " + message + "\n";
        broadcast_message(formatted_msg, client_id, clients, clients_mutex);
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [client_fd](const ClientInfo &c)
                                     { return c.fd == client_fd; }),
                      clients.end());
    }

    close(client_fd);
}

void ClientHandler::broadcast_message(const std::string &message,
                                      const std::string &sender_id,
                                      std::vector<ClientInfo> &clients,
                                      std::mutex &clients_mutex)
{
    std::lock_guard<std::mutex> lock(clients_mutex);

    for (auto it = clients.begin(); it != clients.end();)
    {
        if (it->id != sender_id && it->fd != -1)
        {
            if (send(it->fd, message.c_str(), message.size(), MSG_NOSIGNAL) <= 0)
            {
                close(it->fd);
                it = clients.erase(it);
                continue;
            }
        }
        ++it;
    }
}

bool ClientHandler::is_http_request(const std::string &message)
{
    const std::vector<std::string> http_methods = {
        "GET ", "POST ", "PUT ", "DELETE ", "HEAD ", "OPTIONS ", "PATCH ", "CONNECT ", "TRACE "};

    for (const auto &method : http_methods)
    {
        if (message.find(method) == 0)
        {
            return true;
        }
    }

    const std::vector<std::string> http_headers = {
        "HTTP/1.", "HTTP/2", "Host:", "User-Agent:", "Accept:", "Content-Type:"};

    for (const auto &header : http_headers)
    {
        if (message.find(header) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}