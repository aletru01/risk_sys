#include "parse_msg.hh"
#include <cstddef>

/* 
 * casting or using memcpy for example are not portable because of endianess
 * of different systems. This is the only secure and portable way of doing it.
 */
static uint64_t get_entry(const uint8_t* buffer, std::size_t& offset)
{
    buffer += offset;
    offset += sizeof(uint64_t);

    return (uint64_t)buffer[0] << 56 |
           (uint64_t)buffer[1] << 48 |
           (uint64_t)buffer[2] << 40 |
           (uint64_t)buffer[3] << 32 |
           (uint64_t)buffer[4] << 24 |
           (uint64_t)buffer[5] << 16 |
           (uint64_t)buffer[6] << 8  |
           (uint64_t)buffer[7] << 0;
}

NewOrder parse_new_order(const uint8_t* buffer)
{
    NewOrder new_order;
    std::size_t offset = 0;

    new_order.message_type = 1;
    new_order.listing_id = get_entry(buffer, offset);
    new_order.order_id = get_entry(buffer, offset);
    new_order.order_quantity = get_entry(buffer, offset);
    new_order.order_price = get_entry(buffer, offset);
    new_order.side = buffer[offset];

    return new_order;
}

DeleteOrder parse_del_order(const uint8_t* buffer)
{
    DeleteOrder del_order;
    std::size_t offset = 0;

    del_order.message_type = 2;
    del_order.order_id = get_entry(buffer, offset);

    return del_order;
}

ModifyOrderQuantity parse_modify(const uint8_t* buffer)
{
    ModifyOrderQuantity modify;
    std::size_t offset = 0;

    modify.message_type = 3;
    modify.order_id = get_entry(buffer, offset);
    modify.new_quantity = get_entry(buffer, offset);

    return modify;
}

Trade parse_trade(const uint8_t* buffer)
{
    Trade trade;
    std::size_t offset = 0;
    
    trade.message_type = 4;
    trade.listing_id = get_entry(buffer, offset);
    trade.trade_id = get_entry(buffer, offset);
    trade.trade_quantity = get_entry(buffer, offset);
    trade.trade_price = get_entry(buffer, offset);

    return trade;
}
