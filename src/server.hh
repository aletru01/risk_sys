#pragma once

#include <vector>
#include <mutex>

class Server
{
  public:
    Server();
    int sockfd;

    char buffer[60];
    std::mutex buffer_mutex;
    std::vector<int> clients;
};
