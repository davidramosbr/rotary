#include "common.hpp"
#include <random>

std::string generate_client_id()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    const char *hex_digits = "0123456789ABCDEF";
    std::string id;
    for (int i = 0; i < 8; ++i)
    {
        id += hex_digits[dis(gen)];
    }
    return id;
}