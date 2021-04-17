#pragma once

#include <vector>
#include <mutex>
#include <unordered_map>
#include "parse_msg.hh"

struct Data
{
    int netpos;
    int buyqty;
    int sellqty;
    int buyhypo;
    int sellhypo;
};

class Server
{
  public:
    Server();
    ~Server();
    Server(const Server&) = delete;
    Server& operator= (const Server& server) = delete;
    Server(Server&& server) = delete;
    Server& operator= (Server&& server) = delete;

    int sockfd;
    int buy_threshold;
    int sell_threshold;
    int resp_id;

    uint8_t buffer[60];
    std::mutex buffer_mutex;

    std::unordered_map<int, uint64_t> clientfd_to_listing_id;
    std::unordered_map<uint64_t, Data> listing_id_to_data;
    std::unordered_map<int, NewOrder> orders;

    void erase_client_data(int clientfd);
    void send_response(int success, const std::unordered_map<uint64_t, Data>& previous_map);
    int process_msg(int clientfd);

    void add_order_table(const NewOrder& new_order);
    void delete_order_table(const DeleteOrder& del_order);
    void modify_order_qty(const ModifyOrderQuantity& modify);
    void trade_table(const Trade& t);
    int compute_hypothetical(void);
};

void request_handler(int id, Server& server, int clientfd);
