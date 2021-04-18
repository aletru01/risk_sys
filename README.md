The risk server receives orders and trades (socker messages). After every order, an answer is expected.
It either accepts or rejects the order based on the hypothetical worst net position of every client.

Usage: ./risk_sys <buy_threshold> <sell_threshold>

ex: ./risk_sys 15 20

netcat can be used to send the orders and trades.
