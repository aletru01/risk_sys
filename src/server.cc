#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <vector>
#include <thread>
#include "server.hh"
#include "update_table.hh"

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

int parse_msg(Server& server, int clientfd)
{
    Header header;
    header.version = (server.buffer[0] << 8 ) | server.buffer[1];
    header.payloadSize = (server.buffer[2] << 8 ) | server.buffer[3];

    uint8_t* skipheader = server.buffer + sizeof(Header);
    if (header.payloadSize == sizeof(NewOrder))
    {
        NewOrder neworder = parse_neworder(skipheader);
        server.clientfd_to_listingId[clientfd] = neworder.listingId;
        add_order_table(server, neworder);
    }
    else if (header.payloadSize == sizeof(DeleteOrder))
    {
        DeleteOrder delorder = parse_delorder(skipheader);
        delete_order_table(server, delorder);
        compute_hypothetical(server);
        return -1;
    }
    else if (header.payloadSize == sizeof(ModifyOrderQuantity))
    {
        ModifyOrderQuantity modify = parse_modify(skipheader);
        modify_order_qty(server, modify);
    }
    else if (header.payloadSize == sizeof(Trade))
    {
        Trade trade = parse_trade(skipheader);
        trade_table(server, trade);
        return -1;
    }
    int ret = compute_hypothetical(server);
    return ret;
}

uint8_t* convert_order_to_bytes(OrderResponse order_resp)
{
    uint8_t* response = new uint8_t[sizeof(OrderResponse) + 1];
    response[0] = (static_cast<uint8_t>(order_resp.messageType) >> 8) & 0xFF;
    response[1] = (static_cast<uint8_t>(order_resp.messageType)) & 0xFF;
    
    response[2] = (uint8_t) ((order_resp.orderId >> 56) & 0xFF);
    response[3] = (uint8_t) ((order_resp.orderId >> 48) & 0xFF);
    response[4] = (uint8_t) ((order_resp.orderId >> 40) & 0xFF);
    response[5] = (uint8_t) ((order_resp.orderId >> 32) & 0xFF);
    response[6] = (uint8_t) ((order_resp.orderId >> 24) & 0xFF);
    response[7] = (uint8_t) ((order_resp.orderId >> 16) & 0xFF);
    response[8] = (uint8_t) ((order_resp.orderId >> 8) & 0xFF);
    response[9] = (uint8_t) ((order_resp.orderId >> 0) & 0xFF);

    response[10] = (static_cast<uint8_t>(order_resp.status) >> 8) & 0xFF;
    response[11] = (static_cast<uint8_t>(order_resp.status)) & 0xFF;

    return response;
}

void request_handler([[maybe_unused]] int id, Server& server, int clientfd) 
{
    ssize_t size;
    server.clientfd_to_listingId.emplace(clientfd, 0);
    while (1)
    {
        size = recv(clientfd, server.buffer, sizeof(server.buffer), 0);
        std::unordered_map<uint64_t, Data> map_copy = server.listingId_to_data;
        const std::lock_guard<std::mutex> lock(server.buffer_mutex);
        {
            if (size == 0)
            {
                auto listing = server.clientfd_to_listingId[clientfd];
                auto search = server.listingId_to_data.find(listing);
                if (search != server.listingId_to_data.end())
                {

                    for (auto it = server.orders.begin(), next_it = it; it != server.orders.end(); it = next_it)
                    {
                        ++next_it;
                        if (it->second.listingId == listing)
                            server.orders.erase(it);
                    }   
                    server.listingId_to_data.erase(search);
                    server.clientfd_to_listingId.erase(clientfd);

                }
                break;
            }
            int success = parse_msg(server, clientfd);

            OrderResponse order_resp;
            order_resp.messageType = 5;
            order_resp.orderId = server.respId;

            if (success == 0)
            {
                server.listingId_to_data = map_copy;
                order_resp.status = OrderResponse::Status::REJECTED;
            }
            else if (success == 1)
                order_resp.status = OrderResponse::Status::ACCEPTED;

            uint8_t* response;
            if (success == -1)
                response = nullptr;
            else
                response = convert_order_to_bytes(order_resp);

            for (auto const& [client, listid] : server.clientfd_to_listingId)
                send(client, response, size, MSG_NOSIGNAL);
            delete response;
        }
    }
}
