#pragma once

#include "server.hh"

void add_order_table(Server& server, NewOrder& neworder);
void delete_order_table(Server& server, DeleteOrder& delorder);
void modify_order_qty(Server& server, ModifyOrderQuantity& modify);
void trade_table(Server& server, Trade& t);
int compute_hypothetical(Server& server);
