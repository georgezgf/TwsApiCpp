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
	double roundNum(int share, double num, double minTick);	//round price according to  mintick. Up round for sell, down round for buy
	std::vector<STOCK_ORD> truncLmtPrice(std::vector<STOCK_ORD>stockOrder,bool printMinTick);	//truncate the limit price in stock order vector by minTick. The limit price is read from the csv files
	void FillArrivalPriceParams(Order& baseOrder, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);
	void FillVwapParams(Order& baseOrder, double maxPctVol, std::string startTime, std::string endTime,
		bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue);
	std::vector<STOCK_ORD> genOrder(std::vector<CSV_READ> csvRead,double multiplier, double maxPer,double buyingPower);	//generate order for trade from csv reading data, multiplier is for different accounts
	std::vector<double> orderHedgeCal(std::vector<STOCK_ORD> stockOrd);	//calculate the exposure of SPY and IWM. Need the generated stock order and csv file
	std::vector<STOCK_ORD> genHedgeOrder(double SPXexp, double RUTexp);	//generate hedge orders according to the exposure
	std::vector<double> monitorExp(std::vector<STOCK_ORD> orderList);

	/**********************************************************************************************************/
	/*basic trade functions*/
	void waitForNextValidId();
	int queryNextOrderId();
	std::map<std::string, QUOTE_DATA> queryQuote(std::vector<std::string> tickerList); //query bid, ask, bidsize, asksize real time
	std::map<std::string, QUOTE_DATA> queryClose(std::vector<std::string> tickerList); //query close price
	std::vector<POS> queryPos();
	std::map<int, COMB_OPENORD> queryOrd();
	int queryCash();
	int queryBuyingPower();
	std::string queryPriExch(std::string ticker);
	std::vector<int> sendLmtOrder(std::vector<STOCK_ORD> lmtOrder);
	std::vector<int> sendLOOOrder(std::vector<STOCK_ORD> looOrder);	//limit on open
	std::vector<int> sendAPOrder(std::vector<STOCK_ORD> APOrder, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);
	std::vector<int> sendVWAPOrder(std::vector<STOCK_ORD> VWAPOrder, double maxPctVol, std::string startTime, std::string endTime, bool allowPastEndTime, 
		bool noTakeLiq, bool speedUp, double monetaryValue);
	std::vector<int> modifyLmtOrder(std::map<int, MODIFY_ORD> updateOrder);
	double queryMinTick(std::string ticker);
	std::vector<int> sendAucOrder(std::vector<STOCK_ORD> AucOrder);
	void sendFutureMktOrder(std::string localSymbol, std::string expiryDate, int orderQty);	//local symbol of a specific future defined by IB in its description

	/**********************************************************************************************************/
	/*advanced trade functions*/
	std::vector<int> closeAllPos();
	std::vector<int> openMktLmt(std::vector<STOCK_ORD> lmtOrder);	//Send limit orders around open time according to bid(ask) price for BUY(SELL)
	void updateOrder(std::vector<int> orderIdList, double aggBps, int waitTime_s);
	void closeAllStockAP(double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);	//close all positions using arrival price algo
	void closeAllStockVWAP(double maxPctVol, std::string startTime, std::string endTime,
		bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue);	////close all positions using VWAP
	void closePartAP(std::vector<STOCK_ORD> orderList, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue);	//close certain positions using arrival price algo
	void closePartVWAP(std::vector<STOCK_ORD> orderList, double maxPctVol, std::string startTime, std::string endTime,
		bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue);	//close certain positions using VWAP algo
	std::vector<int> openMktAP(std::vector<STOCK_ORD> stockOrd, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
		bool forceCompletion, bool allowPastTime, double monetaryValue); //submit arrival price orders at open market time
	std::vector<int> openMktVWAP(std::vector<STOCK_ORD> stockOrd, double maxPctVol, std::string startTime, std::string endTime,
		bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue);
	void splitLOOVWAPOrders(std::vector<STOCK_ORD>lyStock,  std::vector<STOCK_ORD>ltStock, double LooPar, double maxPctVol, std::string startTime, std::string endTime, bool allowPastEndTime,
		bool noTakeLiq, bool speedUp, double monetaryValue);	//split lt account orders into LOO and VWAP open market orders. Parameters after the STOCK_ORD are for lt VWAP orders
};


