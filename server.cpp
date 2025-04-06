#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

std::mutex clients_mutex;
std::vector<int> clients;

void broadcast_message(const std::string &message, int sender_fd = -1)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int client_fd : clients)
    {
        if (client_fd != sender_fd)
        {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }
}

void handle_client(int client_fd)
{
    char buffer[1024];
    std::string client_ip;

    // Obter endereço IP do cliente
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_fd, (struct sockaddr *)&addr, &addr_len);
    client_ip = inet_ntoa(addr.sin_addr);

    // Notificar sobre nova conexão
    std::string welcome_msg = "Servidor: Novo usuario conectado (" + client_ip + ")\n";
    broadcast_message(welcome_msg, client_fd);

    while (true)
    {
        int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            // Cliente desconectou
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());

            std::string leave_msg = "Servidor: Usuario (" + client_ip + ") desconectou\n";
            broadcast_message(leave_msg);

            close(client_fd);
            break;
        }

        std::string message(buffer, bytes_received);
        std::string formatted_msg = "[" + client_ip + "]: " + message;

        // Enviar mensagem para todos os clientes (incluindo o remetente)
        broadcast_message(formatted_msg);
    }
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Erro ao criar socket\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Erro ao fazer bind\n";
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        std::cerr << "Erro ao ouvir\n";
        return 1;
    }

    std::cout << "Servidor iniciado na porta 8080. Aguardando conexoes...\n";

    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd < 0)
        {
            std::cerr << "Erro ao aceitar conexao\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_fd);
        }

        std::thread(handle_client, client_fd).detach();
    }

    close(server_fd);
    return 0;
}