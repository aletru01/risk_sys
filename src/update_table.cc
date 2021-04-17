#include "server.hh"

void Server::add_order_table(const NewOrder& new_order)
{
    int buyqty = 0;
    int sellqty = 0;
    auto listingId = new_order.listingId;
    auto search = listingId_to_data.find(listingId);

    if (search != listingId_to_data.end())
    {
        if (new_order.side == 'B')
            listingId_to_data[new_order.listingId].buyqty += new_order.orderQuantity;
        else
            listingId_to_data[new_order.listingId].sellqty += new_order.orderQuantity;
    }
    else
    {
        if (new_order.side == 'B')
            buyqty = new_order.orderQuantity;
        else
            sellqty = new_order.orderQuantity;
    
        Data data = {0, buyqty, sellqty, 0, 0 };
        listingId_to_data.emplace(listingId, data);
    }
    auto orderId = new_order.orderId;
    orders.emplace(orderId, new_order);
    respId = new_order.orderId;
}

void Server::delete_order_table(const DeleteOrder& del_order)
{
    NewOrder order = orders[del_order.orderId];

    if (order.side == 'B')
        listingId_to_data[order.listingId].buyqty -= order.orderQuantity;
    else
        listingId_to_data[order.listingId].sellqty -= order.orderQuantity;

    orders.erase(del_order.orderId);
}

void Server::modify_order_qty(const ModifyOrderQuantity& modify)
{
    NewOrder order = orders[modify.orderId];
    int oldqty = order.orderQuantity;

    if (order.side == 'B')
        listingId_to_data[order.listingId].buyqty += modify.newQuantity - oldqty;
    else
        listingId_to_data[order.listingId].sellqty += modify.newQuantity;
    respId = modify.orderId;
}

void Server::trade_table(const Trade& t)
{
    int64_t quantity = t.tradeQuantity;
    listingId_to_data[t.listingId].netpos += quantity;

    int netpos  = listingId_to_data[t.listingId].netpos;
    int buyqty  = listingId_to_data[t.listingId].buyqty;
    int sellqty = listingId_to_data[t.listingId].sellqty;

    listingId_to_data[t.listingId].buyhypo = std::max(buyqty, netpos + buyqty);
    listingId_to_data[t.listingId].sellhypo = std::max(sellqty, sellqty - netpos);
}

int Server::compute_hypothetical(void)
{
    for (auto& [id, data] : listingId_to_data)
    {
        data.buyhypo = std::max(data.buyqty, data.netpos + data.buyqty);
        data.sellhypo = std::max(data.sellqty, data.sellqty - data.netpos);
        if (data.buyhypo > buy_threshold || 
                data.sellhypo > sell_threshold)
            return 0;
    }
    return 1;
}
