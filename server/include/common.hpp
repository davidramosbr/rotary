#pragma once

#include <string>
#include <vector>
#include <mutex>

struct ClientInfo
{
    int fd;
    std::string id;
    std::string ip;
};

std::string generate_client_id();
std::string get_current_time();