#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include "ctpl.h"
#include "server.hh"

constexpr int THREAD_POOL_NUMBER = 10;

int main(int argc, char** argv)
{
    int clientfd;
    size_t len;
    struct sockaddr_in client_addr;

    if (argc != 3)
    {
        std::cerr << "Usage: ./server <buy_threshold> <sell_threshold>" 
            << std::endl;
        std::cerr << "Example: ./server 15 20" << std::endl;
        exit(1);
    }
    std::string arg1 = argv[1];
    std::string arg2 = argv[2];

    Server server;
    server.buy_threshold = std::stoi(arg1);
    server.sell_threshold = std::stoi(arg2);

    ctpl::thread_pool tp(THREAD_POOL_NUMBER);
    while (true)
    {
        len = sizeof(client_addr); 

        clientfd = accept(server.sockfd, 
                (struct sockaddr*)&client_addr, (socklen_t*)&len); 

        if (clientfd < 0) { 
            perror("accept"); 
            continue; 
        } 
        else
            std::cout << "server acccept the client..." << std::endl;

        tp.push(request_handler, std::ref(server), clientfd);
    }
} 
