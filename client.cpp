#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void receive_messages(SOCKET sock)
{
    char buffer[1024];
    while (true)
    {
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            std::cout << "Desconectado do servidor\n";
            break;
        }
        std::cout << std::string(buffer, bytes_received);
    }
}

int main()
{
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Falha ao inicializar Winsock\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Erro ao criar socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    std::string server_ip;
    std::cout << "Digite o IP do servidor: ";
    std::cin >> server_ip;
    std::cin.ignore(); // Limpar o buffer

    // Converter IP para formato numérico
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0)
    {
        std::cerr << "Endereco invalido\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Conexao falhou: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Conectado ao servidor. Digite mensagens (ou 'sair' para desconectar):\n";

    // Thread para receber mensagens
    std::thread receiver(receive_messages, sock);
    receiver.detach();

    // Envio de mensagens
    std::string message;
    while (true)
    {
        std::getline(std::cin, message);

        if (message == "sair")
        {
            break;
        }

        if (!message.empty())
        {
            send(sock, message.c_str(), message.size(), 0);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}