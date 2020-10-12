#include "parse_msg.hh"
#include <iostream>

uint64_t convert_uint8_to_uint64(uint8_t* buffer)
{
    return (uint64_t)buffer[0] << 56 |
           (uint64_t)buffer[1] << 48 |
           (uint64_t)buffer[2] << 40 |
           (uint64_t)buffer[3] << 32 |
           (uint64_t)buffer[4] << 24 |
           (uint64_t)buffer[5] << 16 |
           (uint64_t)buffer[6] << 8  |
           (uint64_t)buffer[7] << 0;
}

NewOrder parse_neworder(uint8_t* buffer)
{
    NewOrder neworder;
    size_t offset = 0;
    
    neworder.messageType = (buffer[offset] << 8) | buffer[offset+1];
    offset += sizeof(neworder.messageType);
    
    neworder.listingId = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(neworder.listingId);
    
    neworder.orderId = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(neworder.orderId);

    neworder.orderQuantity = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(neworder.orderQuantity);
    
    neworder.orderPrice = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(neworder.orderPrice);

    neworder.side = buffer[offset];

    return neworder;
}

DeleteOrder parse_delorder(uint8_t* buffer)
{
    DeleteOrder delorder;
    size_t offset = 0;

    delorder.messageType = (buffer[offset] << 8) | buffer[offset+1];
    offset += sizeof(delorder.messageType);

    delorder.orderId = convert_uint8_to_uint64(buffer + offset);

    return delorder;
}

ModifyOrderQuantity parse_modify(uint8_t* buffer)
{
    ModifyOrderQuantity modify;
    size_t offset = 0;

    modify.messageType = (buffer[offset] << 8) | buffer[offset+1];
    offset += sizeof(modify.messageType);

    modify.orderId = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(modify.orderId);

    modify.newQuantity = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(modify.newQuantity);

    return modify;
}

Trade parse_trade(uint8_t* buffer)
{
    Trade trade;
    size_t offset = 0;
    
    trade.messageType = (buffer[offset] << 8) | buffer[offset+1];
    offset += sizeof(trade.messageType);
    
    trade.listingId = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(trade.listingId);
    
    trade.tradeId = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(trade.tradeId);

    trade.tradeQuantity = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(trade.tradeQuantity);
    
    trade.tradePrice = convert_uint8_to_uint64(buffer + offset);
    offset += sizeof(trade.tradePrice);

    return trade;
}

