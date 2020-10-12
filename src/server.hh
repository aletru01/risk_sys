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
    int buyside;
    int sellside;
};

class Server
{
  public:
    Server();
    int sockfd;

    uint8_t buffer[60];
    std::mutex buffer_mutex;
    std::vector<int> clients;//a enlever 


    std::unordered_map<int, int> clientfd_to_listingId;
    std::unordered_map<int, Data> listingId_to_Data; //delete information of trader
    //std::vector<Data> traders;
    std::unordered_map<int, NewOrder> Orders; //new, delete, modify; will update Data

};
