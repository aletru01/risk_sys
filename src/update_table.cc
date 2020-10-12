#include "update_table.hh"

void add_order_table(Server& server, NewOrder& neworder)
{
    auto search = server.listingId_to_data.find(neworder.listingId);
    if (search != server.listingId_to_data.end())
    {
        if (neworder.side == 'B')
            server.listingId_to_data[neworder.listingId].buyqty += neworder.orderQuantity;
        else
            server.listingId_to_data[neworder.listingId].sellqty += neworder.orderQuantity;
    }
    else
    {
        int buyqty = 0;
        int sellqty = 0;

        if (neworder.side == 'B')
            buyqty = neworder.orderQuantity;
        else
            sellqty = neworder.orderQuantity;
        Data data = {0, buyqty, sellqty, 0, 0 };

        auto listingId = neworder.listingId;
        server.listingId_to_data.emplace(listingId, data);
    }
    auto orderId = neworder.orderId;
    server.orders.emplace(orderId, neworder);
    server.respId = neworder.orderId;
}

void delete_order_table(Server& server, DeleteOrder& delorder)
{
    NewOrder order = server.orders[delorder.orderId];
    if (order.side == 'B')
        server.listingId_to_data[order.listingId].buyqty -= order.orderQuantity;
    else
        server.listingId_to_data[order.listingId].sellqty -= order.orderQuantity;

    server.orders.erase(delorder.orderId);
}

void modify_order_qty(Server& server, ModifyOrderQuantity& modify)
{
    NewOrder order = server.orders[modify.orderId];
    int oldqty = order.orderQuantity;

    if (order.side == 'B')
        server.listingId_to_data[order.listingId].buyqty += modify.newQuantity - oldqty;
    else
        server.listingId_to_data[order.listingId].sellqty += modify.newQuantity;
    server.respId = modify.orderId;
}

void trade_table(Server& server, Trade& t)
{
    int64_t quantity = t.tradeQuantity;
    server.listingId_to_data[t.listingId].netpos += quantity;

    int netpos  = server.listingId_to_data[t.listingId].netpos;
    int buyqty  = server.listingId_to_data[t.listingId].buyqty;
    int sellqty = server.listingId_to_data[t.listingId].sellqty;

    server.listingId_to_data[t.listingId].buyhypo = std::max(buyqty, netpos + buyqty);
    server.listingId_to_data[t.listingId].buyhypo = std::max(sellqty, sellqty - netpos);
}

int compute_hypothetical(Server& server)
{
    for (auto& [id, data] : server.listingId_to_data)
    {
        data.buyhypo = std::max(data.buyqty, data.netpos + data.buyqty);
        data.sellhypo = std::max(data.sellqty, data.sellqty - data.netpos);
        if (data.buyhypo > server.buy_threshold || 
                data.sellhypo > server.sell_threshold)
            return 0;
    }
    return 1;
}
