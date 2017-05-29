#pragma once

#include "myEWrapper.h"
// "TwsApiL0.h" and "TwsApiDefs.h" are included in "myEWrapper.h"
#include "CSVIterator.h"
#include <map>




class IBAPI {

private:

	std::map<std::string, QUOTE_DATA> quoteMap;

public:

	MyEWrapper EW;
	EClientL0 * EC;

	IBAPI();
	~IBAPI();

	void printInfo(std::string info);
	std::vector<STOCK_ORD> getCSV(std::string str);

	//QUOTE_DATA queryQuote(std::string tickerList);
	std::map<std::string, QUOTE_DATA> queryQuote(std::vector<std::string> tickerList);
	std::vector<STOCK_POS> queryPos();
	std::vector<OPEN_ORD> queryOrd();

	std::vector<int> sendLmtOrder(std::vector<STOCK_ORD> lmtOrder);
	void closeAllPos();
	void openMktLmt(std::vector<STOCK_ORD> lmtOrder);	//Send limit orders around open time according to bid(ask) price for BUY(SELL)
	int queryCash();
};


