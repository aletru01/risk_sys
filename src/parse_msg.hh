#pragma once

#include <cstdint>

struct Header
{
    uint16_t version; // Protocol version
    uint16_t payload_size; // Payload size in bytes
    uint32_t sequenceNumber; // Sequence number for this package
    uint64_t timestamp; // Timestamp, number of nanoseconds from Unix epoch.
} __attribute__ ((__packed__));
static_assert(sizeof(Header) == 16, "The Header size is not correct");

struct NewOrder
{
    static constexpr uint16_t MESSAGE_TYPE = 1;
    uint16_t message_type; // Message type of this message
    uint64_t listing_id; // Financial instrument id associated to this message
    uint64_t order_id; // Order id used for further order changes
    uint64_t order_quantity; // Order quantity
    uint64_t order_price; // Order price, the price contains 4 implicit decimals
    char side; // The side of the order, 'B' for buy and 'S' for sell
} __attribute__ ((__packed__));
static_assert(sizeof(NewOrder) == 35, "The NewOrder size is not correct");

struct DeleteOrder
{
    static constexpr uint16_t MESSAGE_TYPE = 2;
    uint16_t message_type; // Message type of this message
    uint64_t order_id; // Order id that refers to the original order id
} __attribute__ ((__packed__));
static_assert(sizeof(DeleteOrder) == 10, "The DeleteOrder size is not correct");

struct ModifyOrderQuantity
{
    static constexpr uint16_t MESSAGE_TYPE = 3;
    uint16_t message_type; // Message type of this message
    uint64_t order_id; // Order id that refers to the original order id
    uint64_t new_quantity; // The new quantity
} __attribute__ ((__packed__));
static_assert(sizeof(ModifyOrderQuantity) == 18, "The ModifyOrderQuantity size is not correct");

struct Trade
{
    static constexpr uint16_t MESSAGE_TYPE = 4;
    uint16_t message_type; // Message type of this message
    uint64_t listing_id; // Financial instrument id associated to this message
    uint64_t trade_id; // Order id that refers to the original order id
    uint64_t trade_quantity; // Trade quantity
    uint64_t trade_price; // Trade price, the price contains 4 implicit decimals
} __attribute__ ((__packed__));
static_assert(sizeof(Trade) == 34, "The Trade size is not correct");

struct OrderResponse
{
    static constexpr uint16_t MESSAGE_TYPE = 5;
    enum class Status : uint16_t
    {
        ACCEPTED = 0,
        REJECTED = 1,
    };
    uint16_t message_type; // Message type of this message
    uint64_t order_id; // Order id that refers to the original order id
    Status status; // Status of the order
} __attribute__ ((__packed__));
static_assert(sizeof(OrderResponse) == 12, "The OrderResponse size is not correct");

NewOrder parse_new_order(const uint8_t* buffer);
DeleteOrder parse_del_order(const uint8_t* buffer);
ModifyOrderQuantity parse_modify(const uint8_t* buffer);
Trade parse_trade(const uint8_t* buffer);
