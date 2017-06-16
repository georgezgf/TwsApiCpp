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
	std::vector<CSV_READ> getCSV(std::string str);
	double roundNum(double num, double minTick);	//round price to neareast min tick
	void FillArrivalPriceParams(Order& baseOrder, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);
	std::vector<STOCK_ORD> genOrder(std::vector<CSV_READ> csvRead,double multiplier);	//generate order for trade from csv reading data, multiplier is for different accounts


	/**********************************************************************************************************/
	/*basic trade functions*/
	void waitForNextValidId();
	int queryNextOrderId();
	std::map<std::string, QUOTE_DATA> queryQuote(std::vector<std::string> tickerList);
	std::vector<POS> queryPos();
	std::map<int, COMB_OPENORD> queryOrd();
	int queryCash();
	std::string queryPriExch(std::string ticker);
	std::vector<int> sendLmtOrder(std::vector<STOCK_ORD> lmtOrder);
	std::vector<int> sendAPOrder(std::vector<STOCK_ORD> APOrder, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);
	std::vector<int> modifyLmtOrder(std::map<int, MODIFY_ORD> updateOrder);
	double queryMinTick(std::string ticker);


	/**********************************************************************************************************/
	/*advance trade functions*/
	std::vector<int> closeAllPos();
	std::vector<int> openMktLmt(std::vector<STOCK_ORD> lmtOrder);	//Send limit orders around open time according to bid(ask) price for BUY(SELL)
	void updateOrder(std::vector<int> orderIdList, double aggBps, int waitTime_s);
	void closeAllStockAP(double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);	//close all positions using arrival price algo
	std::vector<int> openMktAP(std::vector<STOCK_ORD> stockOrd, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue); //submit arrival price orders at open market time
};


