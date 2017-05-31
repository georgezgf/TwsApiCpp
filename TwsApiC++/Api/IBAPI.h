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

	void waitForNextValidId();
	int queryNextOrderId();
	std::map<std::string, QUOTE_DATA> queryQuote(std::vector<std::string> tickerList);
	std::vector<STOCK_POS> queryPos();
	std::map<int, COMB_OPENORD> queryOrd();
	int queryCash();
	std::map<int, COMB_OPENORD> sendLmtOrder(std::vector<STOCK_ORD> lmtOrder);


	/********************************************************/
	/*The following are more advanced functions*/

	void closeAllPos();
	void openMktLmt(std::vector<STOCK_ORD> lmtOrder);	//Send limit orders around open time according to bid(ask) price for BUY(SELL)
	void updateOrder(std::vector<std::string> ticker, double aggBps);
};


