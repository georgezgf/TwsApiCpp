#pragma once

#include "TwsApiL0.h"
#include "TwsApiDefs.h"
#include "myStruct.h"

#include <iostream>

class MyEWrapper : public EWrapperL0
{
	
public:

	bool b_nextValidId;
	bool b_posReady;
	bool b_openOrdReady;
	bool b_accSummary;
	OrderId m_orderId;
	int n_quote;		// Record how many time tickprice and ticksize functions are called. 
						// For example, if query 3 stocks, m_quoteReady = 3*4(askprice,bidprice,asksize,bidsize)=12 means query quote ready. 

	int n_openOrder;	// For sendLMTorder function, record how many times openOrder function are called by the API

	std::vector<QUOTE_DATA> tickData;	//Record quote data for queryQuote function
	std::vector<std::string> tickerList;
	std::vector<STOCK_POS> stockPos;
	std::vector<OPEN_ORD> openOrd;
	int cashBalance;

	MyEWrapper();

	void waitForNextValidId();
	void init_tickData();	// Initilize the tickData vector to zero

	// Implementation of EwrapperL0

	void tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute);
	void tickSize(TickerId tickerId, TickType field, int size);
	void winError(const IBString& str, int lastError);
	void connectionClosed();
	void updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName);
	void updatePortfolio(const Contract& contract, int position, double marketPrice, double marketValue, double averageCost, \
		double unrealizedPNL, double realizedPNL, const std::string& accountName);
	void position(const IBString& account, const Contract& contract, int position, double avgCost);
	void positionEnd();
	void accountDownloadEnd(const IBString& accountName);
	void nextValidId(OrderId orderId);
	void openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState);
	void orderStatus(OrderId orderId, const IBString& status, int filled, int remaining, double avgFillPrice, int permId, int parentId, \
		double lastFillPrice, int clientId, const IBString& whyHeld);
	void openOrderEnd();
	void contractDetails(const ContractDetails& contractDetails);
	void error(const int id, const int errorCode, const IBString errorString);
	void historicalData(TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps);
	void updateMktDepth(TickerId id, int position, int operation, int side, double price, int size);
	void updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation, int side, double price, int size);
	void connectionOpened(void);
	void checkMessagesStarted(void);
	void checkMessagesStopped(void);
	void accountSummary(int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& currency);
	void accountSummaryEnd(int reqId);

};