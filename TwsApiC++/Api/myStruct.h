#pragma once

// Include customize defined structs		zgf 052317

#include <vector>


struct QUOTE_DATA {
	int bidSize[10];
	double bidPrice[10];
	double askPrice[10];
	int askSize[10];
};

struct OPEN_ORD {
	int OrderId;
	std::string ticker;
	std::string action;
	int totalQty;
};

struct STOCK_ORD {
	std::string ticker;
	double orderPrice;
	int orderQty;
};

struct STOCK_POS {
	std::string ticker;
	double avgCost;
	int posQty;
};
