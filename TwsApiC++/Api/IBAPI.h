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

	/**********************************************************************************************************/
	/*general functions*/
	void printInfo(std::string info);
	std::vector<STOCK_ORD> getCSV(std::string str);
	double roundNum(double num, double minTick);	//round price to neareast min tick


	/**********************************************************************************************************/
	/*basic trade functions*/
	void waitForNextValidId();
	int queryNextOrderId();
	std::map<std::string, QUOTE_DATA> queryQuote(std::vector<std::string> tickerList);
	std::vector<STOCK_POS> queryPos();
	std::map<int, COMB_OPENORD> queryOrd();
	int queryCash();
	std::vector<int> sendLmtOrder(std::vector<STOCK_ORD> lmtOrder);
	std::vector<int> modifyLmtOrder(std::map<int, MODIFY_ORD> updateOrder);
	double queryMinTick(std::string ticker);


	/**********************************************************************************************************/
	/*advance trade functions*/
	std::vector<int> closeAllPos();
	std::vector<int> openMktLmt(std::vector<STOCK_ORD> lmtOrder);	//Send limit orders around open time according to bid(ask) price for BUY(SELL)
	void updateOrder(std::vector<int> orderIdList, double aggBps, int waitTime_s);
};


