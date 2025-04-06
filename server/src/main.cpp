#include "server.hpp"
#include <iostream>

uint SERVER_PORT = 8080;

int main()
{
    try
    {
        Server server(SERVER_PORT);
        std::cout << "Iniciando servidor de chat na porta " << SERVER_PORT << "..." << std::endl;
        server.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}