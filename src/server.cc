#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>
#include "server.hh"

void handler(Server& server, int clientfd) 
{
    ssize_t size;
    server.clients.push_back(clientfd);
    while (1)
    {
        size = recv(clientfd, server.buffer, sizeof(server.buffer), 0);
        const std::lock_guard<std::mutex> lock(server.buffer_mutex);
        {
            std::cout << size << std::endl;
            for (int i = 0; i < server.clients.size(); i++)
                send(server.clients[i], server.buffer, size, MSG_NOSIGNAL);
        }
    }
}

Server::Server()
{
    constexpr uint16_t port = 8080;
    struct sockaddr_in serv_addr;
    int opt = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    bzero(&serv_addr, sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if ((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0)
    { 
        perror("bind");
        exit(1);
    } 

    if ((listen(sockfd, 5)) != 0) 
    { 
        perror("listen");
        exit(1);
    } 
}

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

    int buy_threshold = std::stoi(arg1);
    int sell_threshold = std::stoi(arg2);

    Server server;

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

        threads.push_back(std::thread(&handler, std::ref(server), clientfd));
    }
    for (auto&& t : threads)
    {
        t.join();
    }
    close(server.sockfd); 
} 
