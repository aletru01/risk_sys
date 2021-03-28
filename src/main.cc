#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include "server.hh"
#include <thread>

int main(int argc, char** argv)
{
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

    int clientfd;
    size_t len;
    struct sockaddr_in client_addr;
    std::vector<std::thread> threads;

    while (1)
    {
        len = sizeof(client_addr); 

        clientfd = accept(server.sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&len); 
        if (clientfd < 0) { 
            perror("accept"); 
            continue; 
        } 
        else
            printf("server acccept the client...\n"); 

        threads.push_back(std::thread(&request_handler, std::ref(server), clientfd));
    }
    for (auto&& t : threads)
    {
        t.join();
    }
    close(server.sockfd); 
} 

