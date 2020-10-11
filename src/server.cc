#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>

char buffer[60];
std::mutex buffer_mutex;
std::vector<int> clients;

void handler(int clientfd) 
{
    ssize_t size;
    clients.push_back(clientfd);
    while (1)
    {
        size = recv(clientfd, buffer, sizeof(buffer), 0);
        const std::lock_guard<std::mutex> lock(buffer_mutex);
        {
            std::cout << size << std::endl;
            for (int i = 0; i < clients.size(); i++)
                send(clients[i], buffer, size, MSG_NOSIGNAL);
        }
    }
}

int main()
{
    constexpr uint16_t port = 8080;
    int clientfd, sockfd;
    size_t len;
    struct sockaddr_in serv_addr, client_addr;
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

    std::vector<std::thread> threads;
    while (1)
    {

        len = sizeof(client_addr); 

        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&len); 
        if (clientfd < 0) { 
            perror("accept"); 
            continue; 
        } 
        else
            printf("server acccept the client...\n"); 

        threads.push_back(std::thread(&handler, clientfd));
    }
    for (auto&& t : threads)
    {
        t.join();
    }
    close(sockfd); 
} 
