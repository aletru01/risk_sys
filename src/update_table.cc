#include "server.hh"

void Server::add_order_table(const NewOrder& new_order)
{
    int buyqty = 0;
    int sellqty = 0;
    auto listing_id = new_order.listing_id;
    auto search = listing_id_to_data.find(listing_id);

    if (search != listing_id_to_data.end())
    {
        if (new_order.side == 'B')
            listing_id_to_data[new_order.listing_id].buyqty += new_order.order_quantity;
        else
            listing_id_to_data[new_order.listing_id].sellqty += new_order.order_quantity;
    }
    else
    {
        if (new_order.side == 'B')
            buyqty = new_order.order_quantity;
        else
            sellqty = new_order.order_quantity;
    
        Data data = {0, buyqty, sellqty, 0, 0 };
        listing_id_to_data.emplace(listing_id, data);
    }
    auto order_id = new_order.order_id;
    orders.emplace(order_id, new_order);
    resp_id = new_order.order_id;
}

void Server::delete_order_table(const DeleteOrder& del_order)
{
    NewOrder order = orders[del_order.order_id];

    if (order.side == 'B')
        listing_id_to_data[order.listing_id].buyqty -= order.order_quantity;
    else
        listing_id_to_data[order.listing_id].sellqty -= order.order_quantity;

    orders.erase(del_order.order_id);
}

void Server::modify_order_qty(const ModifyOrderQuantity& modify)
{
    NewOrder order = orders[modify.order_id];
    int oldqty = order.order_quantity;

    if (order.side == 'B')
        listing_id_to_data[order.listing_id].buyqty += modify.new_quantity - oldqty;
    else
        listing_id_to_data[order.listing_id].sellqty += modify.new_quantity;
    resp_id = modify.order_id;
}

void Server::trade_table(const Trade& t)
{
    int64_t quantity = t.trade_quantity;
    listing_id_to_data[t.listing_id].netpos += quantity;

    int netpos  = listing_id_to_data[t.listing_id].netpos;
    int buyqty  = listing_id_to_data[t.listing_id].buyqty;
    int sellqty = listing_id_to_data[t.listing_id].sellqty;

    listing_id_to_data[t.listing_id].buyhypo = std::max(buyqty, netpos + buyqty);
    listing_id_to_data[t.listing_id].sellhypo = std::max(sellqty, sellqty - netpos);
}

int Server::compute_hypothetical(void)
{
    for (auto& [id, data] : listing_id_to_data)
    {
        data.buyhypo = std::max(data.buyqty, data.netpos + data.buyqty);
        data.sellhypo = std::max(data.sellqty, data.sellqty - data.netpos);
        if (data.buyhypo > buy_threshold || 
                data.sellhypo > sell_threshold)
            return 0;
    }
    return 1;
}
