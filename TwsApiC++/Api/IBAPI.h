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
	void FillVwapParams(Order baseOrder, double maxPctVol, std::string startTime, std::string endTime,
		bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue);
	std::vector<STOCK_ORD> genOrder(std::vector<CSV_READ> csvRead,double multiplier, double buyingPower);	//generate order for trade from csv reading data, multiplier is for different accounts
	std::vector<double> csvHedge(std::vector<CSV_READ> csvRead, std::vector<STOCK_ORD> stockOrd);
	void monitorExp(std::vector<STOCK_ORD> orderList);

	/**********************************************************************************************************/
	/*basic trade functions*/
	void waitForNextValidId();
	int queryNextOrderId();
	std::map<std::string, QUOTE_DATA> queryQuote(std::vector<std::string> tickerList);
	std::vector<POS> queryPos();
	std::map<int, COMB_OPENORD> queryOrd();
	int queryCash();
	int queryBuyingPower();
	std::string queryPriExch(std::string ticker);
	std::vector<int> sendLmtOrder(std::vector<STOCK_ORD> lmtOrder);
	std::vector<int> sendAPOrder(std::vector<STOCK_ORD> APOrder, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);
	std::vector<int> sendVWAPOrder(std::vector<STOCK_ORD> VWAPOrder, double maxPctVol, std::string startTime, std::string endTime, bool allowPastEndTime, 
		bool noTakeLiq, bool speedUp, double monetaryValue);
	std::vector<int> modifyLmtOrder(std::map<int, MODIFY_ORD> updateOrder);
	double queryMinTick(std::string ticker);


	/**********************************************************************************************************/
	/*advanced trade functions*/
	std::vector<int> closeAllPos();
	std::vector<int> openMktLmt(std::vector<STOCK_ORD> lmtOrder);	//Send limit orders around open time according to bid(ask) price for BUY(SELL)
	void updateOrder(std::vector<int> orderIdList, double aggBps, int waitTime_s);
	void closeAllStockAP(double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);	//close all positions using arrival price algo
	void closePartAP(std::vector<STOCK_ORD> orderList, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);	//close certain positions using arrival price algo
	void closePartVWAP(std::vector<STOCK_ORD> orderList, double maxPctVol, std::string startTime, std::string endTime,
		bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue);	//close certain positions using VWAP algo
	std::vector<int> openMktAP(std::vector<STOCK_ORD> stockOrd, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue); //submit arrival price orders at open market time
};


