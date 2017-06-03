#pragma once
// Include customize defined structs

#include <vector>


struct QUOTE_DATA {
	int bidSize[10];
	double bidPrice[10];
	double askPrice[10];
	int askSize[10];
};

struct OPEN_ORD {
	//int OrderId;
	std::string ticker;
	std::string action;
	int totalQty;
};

struct ORD_STATUS {
	std::string status;
	int filled;
	int remaining;
	double avgFillPrice;
	double lastFillPrice;
	int clientId;
};

//Combination of OPEN_ORD and ORD_STATUS
struct COMB_OPENORD {
	OPEN_ORD openOrd;
	ORD_STATUS ordStatus;
};

struct STOCK_ORD {
	std::string ticker;
	double orderPrice;
	int orderQty;
};

struct MODIFY_ORD {
	//int orderId;
	std::string ticker;
	double orderPrice;
	int orderQty;
	double minTick;
};

struct STOCK_POS {
	std::string ticker;
	double avgCost;
	int posQty;
};

struct N_QUOTE_STATUS {
	int n_bidprice;
	int n_askprice;
	int n_bidsize;
	int n_asksize;
};