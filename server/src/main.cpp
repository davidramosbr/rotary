#include "server.hpp"
#include <iostream>

uint SERVER_PORT = 13214;

int main()
{
    try
    {
        Server server(SERVER_PORT);
        server.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}