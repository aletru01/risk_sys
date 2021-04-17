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
    header.payload_size = (buffer[2] << 8 ) | buffer[3];
    buffer += sizeof(Header);
    uint16_t message_type = (buffer[0] << 8) | buffer[1];
    buffer += sizeof(message_type);
    return message_type;
}

int Server::process_msg(int clientfd)
{
    Header header;
    int ret;
    const uint8_t* buffer = this->buffer;
    uint16_t message_type = get_metadata_msg(header, buffer);

    if (message_type == 1 && header.payload_size == sizeof(NewOrder))
    {
        NewOrder new_order = parse_new_order(buffer);
        clientfd_to_listing_id[clientfd] = new_order.listing_id;
        add_order_table(new_order);
    }
    else if (message_type == 2 && header.payload_size == sizeof(DeleteOrder))
    {
        DeleteOrder del_order = parse_del_order(buffer);
        delete_order_table(del_order);
        compute_hypothetical();
        return -1;
    }
    else if (message_type == 3 && header.payload_size == sizeof(ModifyOrderQuantity))
    {
        ModifyOrderQuantity modify = parse_modify(buffer);
        modify_order_qty(modify);
    }
    else if (message_type == 4 && header.payload_size == sizeof(Trade))
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

/* 
 * casting or using memcpy for example are not portable because of endianess
 * of different systems. This is the only secure and portable way of doing it.
 */

static void create_response(const OrderResponse& order_resp, 
                            std::array<uint8_t, SIZE>& response)
{
    response[0] = static_cast<uint8_t>((order_resp.message_type) >> 8) & 0xFF;
    response[1] = static_cast<uint8_t>(order_resp.message_type & 0xFF);
    
    response[2] = static_cast<uint8_t>((order_resp.order_id >> 56) & 0xFF);
    response[3] = static_cast<uint8_t>((order_resp.order_id >> 48) & 0xFF);
    response[4] = static_cast<uint8_t>((order_resp.order_id >> 40) & 0xFF);
    response[5] = static_cast<uint8_t>((order_resp.order_id >> 32) & 0xFF);
    response[6] = static_cast<uint8_t>((order_resp.order_id >> 24) & 0xFF);
    response[7] = static_cast<uint8_t>((order_resp.order_id >> 16) & 0xFF);
    response[8] = static_cast<uint8_t>((order_resp.order_id >> 8) & 0xFF);
    response[9] = static_cast<uint8_t>((order_resp.order_id) & 0xFF);

    response[10] = (static_cast<uint8_t>(order_resp.status) >> 8) & 0xFF;
    response[11] = (static_cast<uint8_t>(order_resp.status)) & 0xFF;
}

void Server::erase_client_data(int clientfd)
{
    auto listing = clientfd_to_listing_id[clientfd];
    auto search = listing_id_to_data.find(listing);
    if (search != listing_id_to_data.end())
    {
        auto it = orders.begin();
        auto next_it = it;
        for (; it != orders.end(); it = next_it)
        {
            ++next_it;
            if (it->second.listing_id == listing)
                orders.erase(it);
        }   
        listing_id_to_data.erase(search);
        clientfd_to_listing_id.erase(clientfd);
    }
}

void Server::send_response(int success, 
                           const std::unordered_map<uint64_t, Data>& prev_map)
{
    OrderResponse order_resp;
    order_resp.message_type = 5;
    order_resp.order_id = resp_id;

    if (success == 0)
    {
        listing_id_to_data = prev_map;
        order_resp.status = OrderResponse::Status::REJECTED;
    }
    else if (success == 1)
        order_resp.status = OrderResponse::Status::ACCEPTED;

    std::array<uint8_t, SIZE> response;
    if (success != -1)
        create_response(order_resp, response);

    for (auto const& [client, listid] : clientfd_to_listing_id)
        send(client, response.data(), SIZE, MSG_NOSIGNAL);
}

void request_handler([[maybe_unused]] int id, Server& server, int clientfd) 
{
    ssize_t size;
    server.clientfd_to_listing_id.emplace(clientfd, 0);
    while (true)
    {
        size = recv(clientfd, server.buffer, sizeof(server.buffer), 0);
        std::unordered_map<uint64_t, Data> prev_map = server.listing_id_to_data;
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
