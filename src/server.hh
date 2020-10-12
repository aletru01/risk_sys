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
    Server(const Server&) = delete;

    int sockfd;
    int buy_threshold;
    int sell_threshold;
    int respId;

    uint8_t buffer[60];
    std::mutex buffer_mutex;

    std::unordered_map<int, uint64_t> clientfd_to_listingId;
    std::unordered_map<uint64_t, Data> listingId_to_data;
    std::unordered_map<int, NewOrder> orders;
};
