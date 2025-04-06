#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

bool wait_for_server_response(SOCKET sock)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    // Timeout de 3 segundos
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    int result = select(0, &readfds, NULL, NULL, &timeout);
    if (result > 0)
    {
        char buffer[256];
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            std::cout << buffer;
            return true;
        }
    }
    return false;
}

void receive_messages(SOCKET sock)
{
    char buffer[1024];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
        {
            std::cout << "\nConexão encerrada" << std::endl;
            break;
        }
        buffer[bytes] = '\0';
        std::cout << buffer;
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Falha ao inicializar Winsock" << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Erro ao criar socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "172.235.149.143", &server_addr.sin_addr);

    std::cout << "Conectando ao servidor..." << std::endl;

    if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Erro na conexão: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Conexão TCP estabelecida. Verificando servidor..." << std::endl;

    // 1. Espera pela confirmação do servidor
    if (!wait_for_server_response(sock))
    {
        std::cerr << "Servidor não respondeu. Verifique a conexão." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // 2. Inicia thread para receber mensagens
    std::thread receiver(receive_messages, sock);
    receiver.detach();

    // 3. Loop principal
    std::cout << "Digite mensagens (ou 'sair' para desconectar):" << std::endl;
    std::string message;
    while (true)
    {
        std::getline(std::cin, message);
        if (message == "sair")
            break;
        if (!message.empty())
        {
            send(sock, (message + "\n").c_str(), message.size() + 1, 0);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}