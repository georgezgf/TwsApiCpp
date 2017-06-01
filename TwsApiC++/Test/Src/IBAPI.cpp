#include "IBAPI.h"
#include <time.h>

#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>
#include <iomanip>
#include <algorithm>

IBAPI::IBAPI() :
	EW(), EC(EClientL0::New(&EW)) {}

IBAPI::~IBAPI() {
	if (EC) {
		delete EC;
	}
	std::cout << "EC pointer has been deleted." << std::endl;
}

void IBAPI::printInfo(std::string info) {
	std::cout << std::endl;
	std::cout << std::left << std::setfill('*')<<std::setw(40)<< info <<std::endl;
}

std::vector<STOCK_ORD> IBAPI::getCSV(std::string str) {

	//Assume the csv file format: tick, score, price, share

	std::ifstream file(str);
	std::vector<std::string> row;
	std::vector <STOCK_ORD> orderCSV;
	STOCK_ORD tmpOrd;
	int count = 0;
	/*
	if (!file.is_open()) {
	std::cout << "Cannot open file: " << str << ". Break." << std::endl;
	return "0";
	}
	*/

	for (CSVIterator loop(file); loop != CSVIterator(); ++loop)
	{
		// Get rid of the first row (usually header). Save data from the second row.
		if (count != 0) {
			std::string ticker = (*loop)[0];
			double price = std::stod((*loop)[2]);
			int share = std::stoi((*loop)[1]) > 0 ? std::stoi((*loop)[3]) : -std::stoi((*loop)[3]);

			tmpOrd = { ticker, price, share };

			orderCSV.push_back(tmpOrd);
		}
		count++;
	}

	return orderCSV;
}

void IBAPI::waitForNextValidId() {
	while (!EW.b_nextValidId) {
		std::cout << "nextValidId is not ready." << std::endl;
		Sleep(1000);
	}
	std::cout << "nextValidId = " << EW.m_orderId << std::endl;
}

int IBAPI::queryNextOrderId() {
	printInfo("Query next order Id. ");

	EW.b_nextValidId = false;
	EC->reqIds(-1);
	int count = 0;

	while (!EW.b_nextValidId) {
		Sleep(100);
		count++;
		if (count > 50) {
			std::cout << "Request next order Id time out. Break." << std::endl;
			break;
		}
	}

	return EW.m_orderId;
}


std::map<std::string, QUOTE_DATA> IBAPI::queryQuote(std::vector<std::string> tickerList) {

	printInfo("Query Quote. ");

	quoteMap.clear();
	
	EW.init_tickData();	// Initialize tickdata size and reset n_quote to 0

	for (int i = 0; i < tickerList.size(); i++){
		EC->reqMktData(i, ContractSamples::USStock(tickerList[i]), "", false);
	}

	// Check if every data is read
	bool Ready = false;
	int count = 0;

	while (!Ready) {
		Sleep(100);
		count++;
		for (int i = 0; i < tickerList.size(); i++) {
			// Check if all data in tickData is ready.
			Ready = (EW.n_quote >= tickerList.size()*4);	// 4 items to record for each stock: bidprice, askprice, bidsize, asksize.
			//std::cout << "Ready = " << Ready << std::endl;
			//std::cout << "n_quote = " << EW.n_quote << std::endl;
			//std::cout << "tickerlist size = " << tickerList.size() << std::endl;
		}
		if (count > 100) {
			std::cout << "Request market data time out. Break." << std::endl;
			break;
		}
	}

	for (int i = 0; i < tickerList.size(); i++) {
		EC->cancelMktData(i);
	}

	for (int i = 0; i < tickerList.size(); i++) {
		quoteMap[tickerList[i]] = EW.tickData[i];
	}

	return quoteMap;
	
}

std::vector<STOCK_POS> IBAPI::queryPos() {

	printInfo("Query Position. ");

	EW.stockPos.clear();
	EW.b_posReady = false;

	EC->reqPositions();

	int count = 0;

	while (!EW.b_posReady) {
		Sleep(500);
		count++;
		if (count > 20) {
			std::cout << "Request position time out. Break." << std::endl;
			break;
		}
	}

	EC->cancelPositions();

	return EW.stockPos;
}

std::map<int, COMB_OPENORD> IBAPI::queryOrd() {

	printInfo("Query Open Order. ");

	EW.combOpenOrd.clear();
	EW.b_openOrdReady = false;

	EC->reqOpenOrders();

	int count = 0;

	while (!EW.b_openOrdReady) {
		Sleep(100);
		count++;
		if (count > 20) {
			std::cout << "Request open order time out. Break." << std::endl;
			break;
		}
	}

	if (EW.combOpenOrd.size() == 0) {
		std::cout << "No open order now." << std::endl;
	}

	return EW.combOpenOrd;
}

int IBAPI::queryCash() {

	printInfo("Request account cash balance. ");

	EC->reqAccountSummary(9001, "All", "$LEDGER");

	int count = 0;

	while (!EW.b_accSummary) {
		Sleep(100);
		count++;
		if (count > 100) {
			std::cout << "Request cash balance time out. Break." << std::endl;
			break;
		}
	}

	EC->cancelAccountSummary(9001);

	return EW.cashBalance;
}

std::vector<int> IBAPI::sendLmtOrder(std::vector<STOCK_ORD> lmtOrder) {

	printInfo("Send limit orders. ");
	EW.combOpenOrd.clear();
	EW.b_openOrdReady = false;

	std::string action;
	std::vector<int> orderIdList;
	int count = 0;
	int orderId = queryNextOrderId();

	for (int i = 0; i < lmtOrder.size(); i++) {
		action = lmtOrder[i].orderQty > 0 ? "BUY" : "SELL";
		EC->placeOrder(EW.m_orderId++, ContractSamples::USStock(lmtOrder[i].ticker), OrderSamples::LimitOrder(action, abs(lmtOrder[i].orderQty), lmtOrder[i].orderPrice));
	}

	while (!EW.b_openOrdReady) {
		Sleep(100);
		count++;
		if (count > 20) {
			std::cout << "Request open order time out. Break." << std::endl;
			break;
		}
	}
	
	for (std::map<int, COMB_OPENORD>::iterator it = EW.combOpenOrd.begin(); it != EW.combOpenOrd.end(); ++it) {
		 orderIdList.push_back(it->first);
	}

	std::cout << "Send limit order finish. Submit order size = " << orderIdList.size() << std::endl;

	return orderIdList;
}



/********************************************************/
/*The following are more advanced functions*/


std::vector<int> IBAPI::closeAllPos() {

	printInfo("Close all positions. ");
	
	std::vector<STOCK_ORD> closeOrd;
	STOCK_ORD tmpOrd;
	std::vector<std::string> closeTickerList;
	double closePrice;
	bool zeroPos = true;	// make sure there is position to close.

	std::vector<STOCK_POS> Position = queryPos();

	if (Position.size() == 0) {
		std::cout << "No position hold. Cannot close." << std::endl;
		return { 0 };
	}

	for (int i = 0; i < Position.size(); i++) {
		closeTickerList.push_back(Position[i].ticker);
		if (Position[i].posQty != 0) {
			zeroPos = false;
		}
	}

	if (zeroPos == true) {
		std::cout << "No position hold. Cannot close. zeroPos = " << zeroPos <<  std::endl;
		return { 0 };
	}

	std::map<std::string, QUOTE_DATA> posMap = queryQuote(closeTickerList);

	for (int i = 0; i < Position.size(); i++) {

		if (Position[i].posQty != 0) {
			// posQty>0: buy position. Need to sell at ask price to close; 
			// posQty<0: sell position. Need to buy at bid price to close.
			closePrice = Position[i].posQty > 0 ? posMap[Position[i].ticker].askPrice[0] : posMap[Position[i].ticker].bidPrice[0];

			tmpOrd = { Position[i].ticker, closePrice, -Position[i].posQty };
			std::cout << "Close ticker:" << tmpOrd.ticker << ". Close price:" << tmpOrd.orderPrice << ". Close position: " << tmpOrd.orderQty << std::endl;
			closeOrd.push_back(tmpOrd);
		}
	}

	printInfo("Send close orders. ");

	return sendLmtOrder(closeOrd);

}

std::vector<int> IBAPI::openMktLmt(std::vector<STOCK_ORD> lmtOrder) {

	printInfo("Send open market limit orders. ");

	std::vector<STOCK_ORD> orderForSend = lmtOrder;
	std::vector<std::string> tickerList;
	double tickerPrice;

	for (int i = 0; i < lmtOrder.size(); i++) {
		tickerList.push_back(lmtOrder[i].ticker);
	}

	std::map<std::string, QUOTE_DATA> quoteMap = queryQuote(tickerList);

	for (int i = 0; i < lmtOrder.size(); i++) {

		tickerPrice = lmtOrder[i].orderQty > 0 ? quoteMap[lmtOrder[i].ticker].bidPrice[0] : quoteMap[lmtOrder[i].ticker].askPrice[0];

		orderForSend[i].orderPrice = tickerPrice;

		std::cout << "Order ticker:" << orderForSend[i].ticker << ". Limit price:" << orderForSend[i].orderPrice << ". Order quantity: " << orderForSend[i].orderQty << std::endl;
	}

	return sendLmtOrder(orderForSend);
}



void IBAPI::updateOrder(std::vector<int> orderIdList, double aggBps) {

	printInfo("Update orders. ");
	std::cout << "Input aggBps =" << aggBps << std::endl;

	std::map<int, COMB_OPENORD> allOrd, myOrd;
	std::map<int, COMB_OPENORD>::iterator it0;
	std::vector<std::string> tickerList;	//tickerlist for request the latest bid/ask
	std::vector<STOCK_ORD> orderUpdate;		//stock order for update
	std::map<std::string, QUOTE_DATA> quoteData;

	allOrd = queryOrd();

	//find the remaining open orders in orderIdList from all the request open order
	for (int i = 0; i < orderIdList.size(); i++) {
		it0 = allOrd.find(orderIdList[i]);
		if (it0 != allOrd.end()) {
			myOrd.insert(*it0);
			tickerList.push_back(it0->second.openOrd.ticker);
			int remQty = it0->second.openOrd.action == "BUY" ? it0->second.ordStatus.remaining : -it0->second.ordStatus.remaining;	//if action="BUY", set quantity positive; otherwise negative
			orderUpdate.push_back({ it0->second.openOrd.ticker, -2, remQty});	//set the updateorder price to -1 for now
		}
	}

	std::cout << "Remain open order size: " << myOrd.size() << std::endl;
	
	for (std::map<int, COMB_OPENORD>::iterator it = myOrd.begin(); it != myOrd.end(); ++it) {
		std::cout << it->first << " => " << (it->second).openOrd.ticker << " action:" << (it->second).openOrd.action << " totalQty: " << (it->second).openOrd.totalQty
			<< ". Remaining: " << (it->second).ordStatus.remaining << ". ClientId: " << (it->second).ordStatus.clientId << "\n";
	}

	for (int i = 0; i < orderUpdate.size(); i++) {
		std::cout << "ticker: " << orderUpdate[i].ticker << ". price: " << orderUpdate[i].orderPrice << " quantity: " << orderUpdate[i].orderQty << std::endl;
	}

	quoteData = queryQuote(tickerList);

	for (int i = 0; i < tickerList.size(); i++) {
		//orderQty>0: BUY, update price = bid price + aggbps; orderQty<0: SELL, update price = ask price - aggbps
		orderUpdate[i].orderPrice = orderUpdate[i].orderQty>0 ?	quoteData[tickerList[i]].bidPrice[0] * (1+aggBps/10000) : quoteData[tickerList[i]].askPrice[0] * (1-aggBps/10000);
		std::cout << "ticker: " << orderUpdate[i].ticker << ". update price: " << orderUpdate[i].orderPrice << std::endl;
	}

	/*
	while (myOrd.size() > 0) {
		std::cout << "Remain open order size: " << myOrd.size() << std::endl;

		allOrd = queryOrd();
		for (int i = 0; i < orderIdList.size(); i++) {
			it = allOrd.find(orderIdList[i]);
			myOrd.insert(*it);
		}
		quoteData = queryQuote(tickerList);
	}
	*/

}