#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <vector>
#include "server.hh"

constexpr size_t SIZE = sizeof(OrderResponse);

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

Server::~Server()
{
    close(sockfd);
}

static inline uint16_t get_metadata_msg(Header& header, const uint8_t*& buffer)
{
    header.version = (buffer[0] << 8 ) | buffer[1];
    header.payloadSize = (buffer[2] << 8 ) | buffer[3];
    buffer += sizeof(Header);
    uint16_t messageType = (buffer[0] << 8) | buffer[1];
    buffer += sizeof(messageType);
    return messageType;
}

int Server::process_msg(int clientfd)
{
    Header header;
    int ret;
    const uint8_t* buffer = this->buffer;
    uint16_t messageType = get_metadata_msg(header, buffer);

    if (messageType == 1 && header.payloadSize == sizeof(NewOrder))
    {
        NewOrder neworder = parse_new_order(buffer);
        clientfd_to_listingId[clientfd] = neworder.listingId;
        add_order_table(neworder);
    }
    else if (messageType == 2 && header.payloadSize == sizeof(DeleteOrder))
    {
        DeleteOrder delorder = parse_del_order(buffer);
        delete_order_table(delorder);
        compute_hypothetical();
        return -1;
    }
    else if (messageType == 3 && header.payloadSize == sizeof(ModifyOrderQuantity))
    {
        ModifyOrderQuantity modify = parse_modify(buffer);
        modify_order_qty(modify);
    }
    else if (messageType == 4 && header.payloadSize == sizeof(Trade))
    {
        Trade trade = parse_trade(buffer);
        trade_table(trade);
        return -1;
    }
    else
        return -1;

    ret = compute_hypothetical();
    return ret;
}

static void create_response(const OrderResponse& order_resp, 
                            std::array<uint8_t, SIZE>& response)
{
    response[0] = static_cast<uint8_t>((order_resp.messageType) >> 8) & 0xFF;
    response[1] = static_cast<uint8_t>(order_resp.messageType & 0xFF);
    
    response[2] = static_cast<uint8_t>((order_resp.orderId >> 56) & 0xFF);
    response[3] = static_cast<uint8_t>((order_resp.orderId >> 48) & 0xFF);
    response[4] = static_cast<uint8_t>((order_resp.orderId >> 40) & 0xFF);
    response[5] = static_cast<uint8_t>((order_resp.orderId >> 32) & 0xFF);
    response[6] = static_cast<uint8_t>((order_resp.orderId >> 24) & 0xFF);
    response[7] = static_cast<uint8_t>((order_resp.orderId >> 16) & 0xFF);
    response[8] = static_cast<uint8_t>((order_resp.orderId >> 8) & 0xFF);
    response[9] = static_cast<uint8_t>((order_resp.orderId) & 0xFF);

    response[10] = (static_cast<uint8_t>(order_resp.status) >> 8) & 0xFF;
    response[11] = (static_cast<uint8_t>(order_resp.status)) & 0xFF;
}

void Server::erase_client_data(int clientfd)
{
    auto listing = clientfd_to_listingId[clientfd];
    auto search = listingId_to_data.find(listing);
    if (search != listingId_to_data.end())
    {
        auto it = orders.begin();
        auto next_it = it;
        for (; it != orders.end(); it = next_it)
        {
            ++next_it;
            if (it->second.listingId == listing)
                orders.erase(it);
        }   
        listingId_to_data.erase(search);
        clientfd_to_listingId.erase(clientfd);
    }
}

void Server::send_response(int success, 
                           const std::unordered_map<uint64_t, Data>& prev_map)
{
    OrderResponse order_resp;
    order_resp.messageType = 5;
    order_resp.orderId = respId;

    if (success == 0)
    {
        listingId_to_data = prev_map;
        order_resp.status = OrderResponse::Status::REJECTED;
    }
    else if (success == 1)
        order_resp.status = OrderResponse::Status::ACCEPTED;

    std::array<uint8_t, SIZE> response;
    if (success != -1)
        create_response(order_resp, response);

    for (auto const& [client, listid] : clientfd_to_listingId)
        send(client, response.data(), SIZE, MSG_NOSIGNAL);
}

void request_handler([[maybe_unused]] int id, Server& server, int clientfd) 
{
    ssize_t size;
    server.clientfd_to_listingId.emplace(clientfd, 0);
    while (true)
    {
        size = recv(clientfd, server.buffer, sizeof(server.buffer), 0);
        std::unordered_map<uint64_t, Data> prev_map = server.listingId_to_data;
        const std::lock_guard<std::mutex> lock(server.buffer_mutex);
        {
            if (size == 0)
            {
                server.erase_client_data(clientfd);
                break;
            }

            int success = server.process_msg(clientfd);
            server.send_response(success, prev_map);
        }
    }
    close(clientfd);
}
