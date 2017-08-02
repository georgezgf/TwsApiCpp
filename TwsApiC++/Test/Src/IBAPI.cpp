#define NOMINMAX

#include "IBAPI.h"
#include <time.h>

#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <math.h>



IBAPI::IBAPI() :
	EW(), EC(EClientL0::New(&EW)) {}

IBAPI::~IBAPI() {
	if (EC) {
		delete EC;
	}
	std::cout << "EC pointer has been deleted." << std::endl;
}

/**********************************************************************************************************/
/*General functions*/

void IBAPI::printInfo(std::string info) {
	std::cout << std::endl;
	std::cout << std::left << std::setfill('*')<<std::setw(40)<< info <<std::endl;
}

std::vector<CSV_READ> IBAPI::getCSV(std::string str) {

	//Assume the csv file format: tick, score, price, dmv

	printInfo("CSV Read.  ");

	std::cout << "File name: " << str << std::endl;

	std::ifstream file(str);
	std::vector<std::string> row;
	std::vector <CSV_READ> CSVRead;
	CSV_READ tmpCSVRead;
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
			double score = std::stod((*loop)[1]);
			double price = std::stod((*loop)[2]);
			int dmv = std::stoi((*loop)[3]);
			double nnlsSPX = std::stod((*loop)[4]);
			double nnlsRUT = std::stod((*loop)[5]);

			tmpCSVRead = { ticker, score, price, dmv, nnlsSPX, nnlsRUT };

			CSVRead.push_back(tmpCSVRead);
		}
		count++;
	}

	return CSVRead;
}

double IBAPI::roundNum(double num, double minTick) {
	double tmpMinTick;

	if (minTick <= 0.05) {
		tmpMinTick = 0.05;
	}
	else {
		tmpMinTick = minTick;
	}

	int inum = int(round(num * 100));
	int iminTick = int(tmpMinTick * 100);
	
	int remain = inum % iminTick;

	//std::cout << "remain = " << remain << std::endl;

	if (remain == 0) {
		return double(inum) / 100;
	}
	
	inum = remain < double(iminTick) / 2 ? inum - remain : inum + iminTick - remain;
	return double(inum) / 100;
	
}

void IBAPI::FillArrivalPriceParams(Order& baseOrder, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
	bool forceCompletion, bool allowPastTime, double monetaryValue) {
	baseOrder.algoStrategy = "ArrivalPx";
	baseOrder.algoParams.reset(new TagValueList());
	TagValueSPtr tag1(new TagValue("maxPctVol", std::to_string(maxPctVol)));
	TagValueSPtr tag2(new TagValue("riskAversion", riskAversion));
	TagValueSPtr tag3(new TagValue("startTime", startTime));
	TagValueSPtr tag4(new TagValue("endTime", endTime));
	TagValueSPtr tag5(new TagValue("forceCompletion", forceCompletion ? "1" : "0"));
	TagValueSPtr tag6(new TagValue("allowPastEndTime", allowPastTime ? "1" : "0"));
	TagValueSPtr tag7(new TagValue("monetaryValue", std::to_string(monetaryValue)));
	baseOrder.algoParams->push_back(tag1);
	baseOrder.algoParams->push_back(tag2);
	baseOrder.algoParams->push_back(tag3);
	baseOrder.algoParams->push_back(tag4);
	baseOrder.algoParams->push_back(tag5);
	baseOrder.algoParams->push_back(tag6);
	baseOrder.algoParams->push_back(tag7);
}

void IBAPI::FillVwapParams(Order& baseOrder, double maxPctVol, std::string startTime, std::string endTime, bool allowPastEndTime, 
	bool noTakeLiq, bool speedUp, double monetaryValue){
	baseOrder.algoStrategy = "Vwap";
	baseOrder.algoParams.reset(new TagValueList());
	TagValueSPtr tag1(new TagValue("maxPctVol", std::to_string(maxPctVol)));
	TagValueSPtr tag2(new TagValue("startTime", startTime));
	TagValueSPtr tag3(new TagValue("endTime", endTime));
	TagValueSPtr tag4(new TagValue("allowPastEndTime", allowPastEndTime ? "1" : "0"));
	TagValueSPtr tag5(new TagValue("noTakeLiq", noTakeLiq ? "1" : "0"));
	TagValueSPtr tag6(new TagValue("speedUp", speedUp ? "1" : "0"));
	TagValueSPtr tag7(new TagValue("monetaryValue", std::to_string(monetaryValue)));
	baseOrder.algoParams->push_back(tag1);
	baseOrder.algoParams->push_back(tag2);
	baseOrder.algoParams->push_back(tag3);
	baseOrder.algoParams->push_back(tag4);
	baseOrder.algoParams->push_back(tag5);
	baseOrder.algoParams->push_back(tag6);
	baseOrder.algoParams->push_back(tag7);
}

std::vector<STOCK_ORD> IBAPI::genOrder(std::vector<CSV_READ> csvRead, double multiplier, double buyingPower) {
	printInfo("Generate orders from CSV reading. ");

	std::vector<STOCK_ORD> stockOrder;

	double tradeValue = 0;

	std::cout << "Input CSVlist size = " << csvRead.size() << std::endl;

	struct {
		bool operator()(CSV_READ &a, CSV_READ &b) const
		{
			return abs(a.score) > abs(b.score);
		}
	} compareScore;

	//sort the vector by the score from high to low
	std::sort(csvRead.begin(), csvRead.end(), compareScore);

	for (int i = 0; i < csvRead.size(); i++) {
		int share = 0;
		
		std::smatch m;
		std::regex r("\\.(\\w)");
		std::string primaryExch;

		//extract the primary exchange
		if (std::regex_search(csvRead[i].ticker, m, r)) {
			//std::cout << "Ticker primary exchange = " << csvRead[i].ticker << ". size = " << m.str(1) << std::endl;
			char exch = m.str(1)[0];
			//std::cout << "exch: " << exch << std::endl;
			
			switch (exch)
			{
			case 'O':
				primaryExch = "ISLAND";
				break;
			case 'N':
				primaryExch = "NYSE";
				break;
			case 'A':
				primaryExch = "AMEX";
				break;
			default:
				primaryExch = "";
			}
		}
		//std::cout << "ticker: " << csvRead[i].ticker << ". Score: " << csvRead[i].score << std::endl;

		std::regex r1("\"|\\..");	//get rid of ".X" and "" for ticker in the original CSV file
									//std::cout << std::regex_replace(tmpticker, r1, "") << std::endl;
		std::string ticker = std::regex_replace(csvRead[i].ticker, r1, "");

		if (abs(csvRead[i].score) >= 40 && abs(csvRead[i].score) < 45) {
			double tmp = std::min(multiplier*10000 / csvRead[i].price, 0.002*csvRead[i].dmv);
			int tmpshare = int(trunc(tmp / 100) * 100);	//down round share to 100
			share = csvRead[i].score > 0 ? tmpshare : -tmpshare;
			//std::cout << tmp << ". tmpshare = " << tmpshare << ". share = " << share <<std::endl;
		}
		else if(abs(csvRead[i].score) >= 45 && abs(csvRead[i].score) < 60){
			double tmp = std::min(multiplier * 10000 / csvRead[i].price, 0.002*csvRead[i].dmv);
			int tmpshare = int(trunc(tmp / 100) * 100);	//down round share to 100
			share = csvRead[i].score > 0 ? tmpshare : -tmpshare;
			//std::cout << tmp << ". tmpshare = " << tmpshare << ". share = " << share << std::endl;
		}
		else if(abs(csvRead[i].score) >= 60) {
			double tmp = std::min(multiplier * 10000 / csvRead[i].price, 0.002*csvRead[i].dmv);
			int tmpshare = int(trunc(tmp / 100) * 100);	//down round share to 100
			share = csvRead[i].score > 0 ? tmpshare : -tmpshare;
			//std::cout << tmp << ". tmpshare = " << tmpshare << ". share = " << share << std::endl;
		}
		if (share != 0 && csvRead[i].price>=3) {
			stockOrder.push_back({ ticker, csvRead[i].price, share, primaryExch,csvRead[i].nnlsSPX,csvRead[i].nnlsRUT });
			tradeValue += csvRead[i].price*abs(share);
		}
		
	}
	std::cout << "Generate orderlist size = " << stockOrder.size() << std::endl;
	std::cout << "Total value of stocks = " << tradeValue << std::endl;


	//if stock value is larger than the total buying power, reduce the multiplier
	if (tradeValue > buyingPower) {
		std::cout << "Buying power = " << buyingPower << ", smaller than the total trade value. Adjust the multiplier." << std::endl;
		double factor = buyingPower / tradeValue ;
		std::cout << "New factor = " << factor << ".   " << std::endl;
		double updateTradeValue = 0;

		//if the factor is less than 0.67, trade less stocks (remove lower score stock) instead of reducing the factor
		if (factor < 0.67) {
			std::cout << "The new factor is less than 0.67. Fix the factor at 0.67, and remove from the lowest score stock until the total trade value < buying power." << std::endl;
			
			//Scale the order qty by the fixed factor 0.67
			for (int i = 0; i < stockOrder.size(); i++) {
				int tmpQty = abs(stockOrder[i].orderQty*0.67);
				int sign = stockOrder[i].orderQty > 0 ? 1 : -1;
				tmpQty = sign * int(trunc(tmpQty / 100) * 100);	//round to 100 stocks
				//stockOrder[i].orderQty = stockOrder[i].orderQty * 0.67;
				//std::cout << "new orderQty = " << stockOrder[i].orderQty << ". tmpQty = " << tmpQty << std::endl;
				stockOrder[i].orderQty = tmpQty;
				updateTradeValue += stockOrder[i].orderPrice * abs(stockOrder[i].orderQty);
			}

			//double targetTradeValue = tradeValue*0.67;
			//std::cout << "Update trade value = " << targetTradeValue << std::endl;

			//remove from the lowest score stock one by one until the update trade value is less than 95% of the buying power
			while (updateTradeValue > 0.9 * buyingPower) {
				std::cout << "Remove stock: " << stockOrder.back().ticker << std::endl;
				updateTradeValue -= stockOrder.back().orderPrice * abs(stockOrder.back().orderQty);
				std::cout << "Update trade value = " << updateTradeValue << ". Order size=" << stockOrder.size() <<std::endl;
				stockOrder.pop_back();
			}
		}
		else {
			for (int i = 0; i < stockOrder.size(); i++) {
				int tmpQty = abs(stockOrder[i].orderQty*factor);
				int sign = stockOrder[i].orderQty > 0 ? 1 : -1;
				tmpQty = sign * int(trunc(tmpQty / 100) * 100);	//round to 100 stocks
				//stockOrder[i].orderQty = stockOrder[i].orderQty * factor;
				//std::cout << "new orderQty = " << stockOrder[i].orderQty << ". tmpQty = " << tmpQty << std::endl;
				stockOrder[i].orderQty = tmpQty;
				updateTradeValue += stockOrder[i].orderPrice * abs(stockOrder[i].orderQty);
			}
		}
		
		std::cout << "Update trade value = " << updateTradeValue << std::endl;
	
	}
	//end stock value restriction.
	
	
	return stockOrder;
}

std::vector<double> IBAPI::orderHedgeCal(std::vector<STOCK_ORD> stockOrd) {
	printInfo("Calculate hedge exposure. ");

	std::vector<double> exposure;

	double SPXexp=0;
	double RUTexp=0;

	for (int i = 0; i < stockOrd.size(); i++) {
		SPXexp += stockOrd[i].orderQty * stockOrd[i].orderPrice *stockOrd[i].nnlsSPX;
		RUTexp += stockOrd[i].orderQty * stockOrd[i].orderPrice*stockOrd[i].nnlsRUT;
	}


	std::cout << "SPX exposure:" << SPXexp << ". RUT exposure: " << RUTexp << std::endl;

	exposure.push_back(SPXexp);
	exposure.push_back(RUTexp);

	return exposure;
}


std::vector<STOCK_ORD> IBAPI::genHedgeOrder(double SPXexp, double RUTexp) {
	printInfo("Generate hedge orders. ");

	std::vector<STOCK_ORD> hedgeOrder;
	std::vector<std::string> hedgeList{ "SPY", "IWM" };
	std::map<std::string,QUOTE_DATA>hedgePrice = queryClose(hedgeList);

	double SPYclose = hedgePrice["SPY"].close[0];
	double IWMclose = hedgePrice["IWM"].close[0];

	std::cout << "SPX close: " << SPYclose << ". RUT close: " << IWMclose << std::endl;

	if (SPYclose <= 0 | IWMclose <= 0) {
		std::cout << "Get hedge close price failed. Use default value." << std::endl;
		SPYclose = 247;
		IWMclose = 142;
	}

	int SPYqty = -int(trunc(SPXexp / SPYclose / 100) * 100);
	int IWMqty = -int(trunc(RUTexp / IWMclose / 100) * 100);

	hedgeOrder.push_back({ "SPY", SPYclose, SPYqty });
	hedgeOrder.push_back({ "IWM", IWMclose, IWMqty });

	std::cout << "SPX hedge qty: " << SPYqty << ". RUT hedge qty: " << IWMqty << std::endl;

	return hedgeOrder;
}


/**********************************************************************************************************/
/*basic trade functions*/

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


double IBAPI::queryMinTick(std::string ticker) {

	printInfo("Query minimum tick. ");

	EW.minTick=0;
	EW.b_ctrDetail = false;

	//int reqId = 8000;
	int count = 0;

	EC->reqContractDetails(0, ContractSamples::USStock(ticker));

	while (!EW.b_ctrDetail) {
		Sleep(100);
		count++;
		if (count > 50) {
			std::cout << "Request minimum tick time out. Break." << std::endl;
			break;
		}
	}
	return EW.minTick;
}

std::string IBAPI::queryPriExch(std::string ticker) {

	printInfo("Query primary exchange. ");

	EW.primaryExchange = "";
	EW.b_ctrDetail = false;

	//int reqId = 8000;
	int count = 0;

	EC->reqContractDetails(1, ContractSamples::USStock(ticker));

	while (!EW.b_ctrDetail) {
		Sleep(100);
		count++;
		if (count > 50) {
			std::cout << "Request exchange time out. Break." << std::endl;
			break;
		}
	}
	return EW.primaryExchange;
}

std::map<std::string, QUOTE_DATA> IBAPI::queryQuote(std::vector<std::string> tickerList) {

	printInfo("Query Quote ask, bid, asksize, bidsize. ");

	quoteMap.clear();
	
	EW.init_tickData();	// Initialize tickdata size and reset n_quote to 0

	for (int i = 0; i < tickerList.size(); i++){
		EC->reqMktData(i, ContractSamples::USStock(tickerList[i]), "", false);
	}

	// Check if every data is read
	bool Ready = false;
	bool tmp;
	int count = 0;

	while (!Ready) {
		Ready = true;
		Sleep(100);
		count++;
		for (int i = 0; i < tickerList.size(); i++) {
			// Check if all data in tickData is ready. For every tickerId, need every entry(askprice, bidprice, asksize, bidsize) to be called at least once
			tmp = (EW.n_quoteStatus[i].n_askprice > 0 & EW.n_quoteStatus[i].n_bidprice > 0 & EW.n_quoteStatus[i].n_asksize > 0 & EW.n_quoteStatus[i].n_bidsize > 0);
			
			//std::cout <<"tickerId = " << i << ". n_askprice, n_bidprice, n_asksize, n_bidsize = " << EW.n_quoteStatus[i].n_askprice << EW.n_quoteStatus[i].n_bidprice
				//<< EW.n_quoteStatus[i].n_asksize << EW.n_quoteStatus[i].n_bidsize << std::endl;
			Ready = Ready & tmp;
		}
		//std::cout << "Ready = " << Ready << std::endl;

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

std::map<std::string, QUOTE_DATA> IBAPI::queryClose(std::vector<std::string> tickerList) {

	printInfo("Query close price. ");

	quoteMap.clear();

	EW.init_tickData();	// Initialize tickdata size and reset n_quote to 0

	for (int i = 0; i < tickerList.size(); i++) {
		EC->reqMktData(i, ContractSamples::USStock(tickerList[i]), "", false);
	}

	// Check if every data is read
	bool Ready = false;
	bool tmp;
	int count = 0;

	while (!Ready) {
		Ready = true;
		Sleep(100);
		count++;
		for (int i = 0; i < tickerList.size(); i++) {
			// Check if all data in tickData is ready. For every tickerId, need every entry(askprice, bidprice, asksize, bidsize) to be called at least once
			tmp = ( EW.n_quoteStatus[i].n_close > 0);

			//std::cout <<"tickerId = " << i << ". n_askprice, n_bidprice, n_asksize, n_bidsize = " << EW.n_quoteStatus[i].n_askprice << EW.n_quoteStatus[i].n_bidprice
			//<< EW.n_quoteStatus[i].n_asksize << EW.n_quoteStatus[i].n_bidsize << std::endl;
			Ready = Ready & tmp;
		}
		//std::cout << "Ready = " << Ready << std::endl;

		if (count > 100) {
			std::cout << "Request close price time out. Break." << std::endl;
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

std::vector<POS> IBAPI::queryPos() {

	printInfo("Query Position. ");

	EW.allPos.clear();
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

	return EW.allPos;
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

	EW.b_accSummary = false;

	EC->reqAccountSummary(9001, "All", "TotalCashValue");

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

int IBAPI::queryBuyingPower() {

	printInfo("Request account buying power. ");

	EW.b_accSummary = false;

	EC->reqAccountSummary(9002, "All", "BuyingPower");

	int count = 0;

	while (!EW.b_accSummary) {
		Sleep(100);
		count++;
		if (count > 100) {
			std::cout << "Request buying power time out. Break." << std::endl;
			break;
		}
	}

	EC->cancelAccountSummary(9002);

	return EW.buyingPower;
}

void IBAPI::monitorExp(std::vector<STOCK_ORD> orderList) {
	printInfo("Monitor index exposure");

	std::vector<std::string> orderTicker;
	std::vector<std::string>::iterator it;
	std::map<std::string, STOCK_ORD> orderMap;
	bool zeroPos = true;

	double SPXexp = 0;
	double RUTexp = 0;

	std::vector<POS> Position = queryPos();
	

	if (Position.size() == 0) {
		std::cout << "No position hold." << std::endl;
		return;
	}

	for (int i = 0; i < orderList.size(); i++) {
		orderTicker.push_back(orderList[i].ticker);
		orderMap[orderList[i].ticker] = orderList[i];	//convert the stock order to ticker->stockorder map
	}

	//Exctract positions that are in the order ticker lists to generate close orders
	for (int i = 0; i < Position.size(); i++) {
		it = std::find(orderTicker.begin(), orderTicker.end(), Position[i].ticker);
		if (Position[i].posQty != 0 && it != orderTicker.end()) {
			//std::string primaryExchange = queryPriExch(Position[i].ticker);
			// posQty>0: buy position. Need to sell at ask price to close; 
			// posQty<0: sell position. Need to buy at bid price to close.

			SPXexp += Position[i].posQty * Position[i].avgCost *orderMap[*it].nnlsSPX;
			RUTexp += Position[i].posQty * Position[i].avgCost *orderMap[*it].nnlsRUT;

			zeroPos = false;
		}

	}

	if (zeroPos == true) {
		std::cout << "No position to calculate exposure." << zeroPos << std::endl;
		return;
	}

	std::cout << "SPX exposure:" << SPXexp << ". RUT exposure: " << RUTexp << std::endl;
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
		EC->placeOrder(EW.m_orderId++, ContractSamples::USStock(lmtOrder[i].ticker,lmtOrder[i].primaryExch), OrderSamples::LimitOrder(action, abs(lmtOrder[i].orderQty), lmtOrder[i].orderPrice));
	}

	while (!EW.b_openOrdReady) {
		Sleep(100);
		count++;
		if (count > 20) {
			std::cout << "Open order call back time out. Break." << std::endl;
			break;
		}
	}
	
	for (std::map<int, COMB_OPENORD>::iterator it = EW.combOpenOrd.begin(); it != EW.combOpenOrd.end(); ++it) {
		 orderIdList.push_back(it->first);
	}

	std::cout << "Send limit order finish. Submit order size = " << orderIdList.size() << std::endl;

	return orderIdList;
}

std::vector<int> IBAPI::sendAPOrder(std::vector<STOCK_ORD> APOrder, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
	bool forceCompletion, bool allowPastTime, double monetaryValue) {

	printInfo("Send Arrival price orders. ");
	EW.combOpenOrd.clear();
	EW.b_openOrdReady = false;

	std::string action;
	std::vector<int> orderIdList;
	int count = 0;
	int orderId = queryNextOrderId();

	for (int i = 0; i < APOrder.size(); i++) {
		action = APOrder[i].orderQty > 0 ? "BUY" : "SELL";
		Order baseOrder = OrderSamples::MarketOrder(action, abs(APOrder[i].orderQty));
		FillArrivalPriceParams(baseOrder, maxPctVol, riskAversion,startTime, endTime, forceCompletion, allowPastTime, monetaryValue);
		EC->placeOrder(EW.m_orderId++, ContractSamples::USStock(APOrder[i].ticker, APOrder[i].primaryExch), baseOrder);
	}

	while (!EW.b_openOrdReady) {
		Sleep(100);
		count++;
		if (count > 20) {
			std::cout << "Open order call back time out. Break." << std::endl;
			break;
		}
	}

	for (std::map<int, COMB_OPENORD>::iterator it = EW.combOpenOrd.begin(); it != EW.combOpenOrd.end(); ++it) {
		orderIdList.push_back(it->first);
	}

	std::cout << "Send Arrival price order finish. Submit order size = " << orderIdList.size() << std::endl;

	return orderIdList;
}

std::vector<int> IBAPI::sendVWAPOrder(std::vector<STOCK_ORD> VWAPOrder, double maxPctVol, std::string startTime, std::string endTime, bool allowPastEndTime,
	bool noTakeLiq, bool speedUp, double monetaryValue) {
	printInfo("Send VWAP orders. ");
	EW.combOpenOrd.clear();
	EW.b_openOrdReady = false;

	std::string action;
	std::vector<int> orderIdList;
	int count = 0;
	int orderId = queryNextOrderId();

	for (int i = 0; i < VWAPOrder.size(); i++) {
		action = VWAPOrder[i].orderQty > 0 ? "BUY" : "SELL";
		Order baseOrder = OrderSamples::MarketOrder(action, abs(VWAPOrder[i].orderQty));
		FillVwapParams(baseOrder, maxPctVol,startTime,endTime,allowPastEndTime,noTakeLiq,speedUp,monetaryValue);
		//FillArrivalPriceParams(baseOrder, maxPctVol, riskAversion, startTime, endTime, forceCompletion, allowPastTime, monetaryValue);
		EC->placeOrder(EW.m_orderId++, ContractSamples::USStock(VWAPOrder[i].ticker, VWAPOrder[i].primaryExch), baseOrder);
	}

	while (!EW.b_openOrdReady) {
		Sleep(100);
		count++;
		if (count > 20) {
			std::cout << "Open order call back time out. Break." << std::endl;
			break;
		}
	}

	for (std::map<int, COMB_OPENORD>::iterator it = EW.combOpenOrd.begin(); it != EW.combOpenOrd.end(); ++it) {
		orderIdList.push_back(it->first);
	}

	std::cout << "Send VWAP order finish. Submit order size = " << orderIdList.size() << std::endl;

	return orderIdList;
}


std::vector<int> IBAPI::modifyLmtOrder(std::map<int, MODIFY_ORD> updateOrd) {

	printInfo("Update limit orders. ");
	EW.combOpenOrd.clear();
	EW.b_openOrdReady = false;

	std::string action;
	std::vector<int> orderIdList;
	int count = 0;

	for (std::map<int, MODIFY_ORD>::iterator it = updateOrd.begin(); it != updateOrd.end(); ++it) {
		action = it->second.orderQty > 0 ? "BUY" : "SELL";
		EC->placeOrder(it->first, ContractSamples::USStock(updateOrd[it->first].ticker), OrderSamples::LimitOrder(action, abs(updateOrd[it->first].orderQty), updateOrd[it->first].orderPrice));
	}

	while (!EW.b_openOrdReady) {
		Sleep(100);
		count++;
		if (count > 20) {
			std::cout << "Open order call back time out. Modify limit orders fail. Break." << std::endl;
			break;
		}
	}

	for (std::map<int, COMB_OPENORD>::iterator it = EW.combOpenOrd.begin(); it != EW.combOpenOrd.end(); ++it) {
		orderIdList.push_back(it->first);
	}

	std::cout << "Update limit order finish. Submit order size = " << orderIdList.size() << std::endl;

	return orderIdList;
}


/**********************************************************************************************************/
/*advance trade functions*/


std::vector<int> IBAPI::closeAllPos() {

	printInfo("Close all positions. ");
	
	std::vector<STOCK_ORD> closeOrd;
	STOCK_ORD tmpOrd;
	std::vector<std::string> closeTickerList;
	double closePrice;
	bool zeroPos = true;	// make sure there is position to close.

	std::vector<POS> Position = queryPos();

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
			std::string primaryExchange = queryPriExch(Position[i].ticker);
			closePrice = Position[i].posQty > 0 ? posMap[Position[i].ticker].askPrice[0] : posMap[Position[i].ticker].bidPrice[0];

			tmpOrd = { Position[i].ticker, closePrice, -Position[i].posQty,primaryExchange };
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



void IBAPI::updateOrder(std::vector<int> orderIdList, double aggBps, int waitTime_s) {

	printInfo("Update orders. ");
	std::cout << "Input aggBps =" << aggBps << ".\n" << "Input orderIdList for update: " << std::endl;
	for (int i = 0; i < orderIdList.size(); i++) {
		std::cout << orderIdList[i] << " ";
	}
	std::cout << std::endl;

	std::vector<int> myOrderIdList;
	std::map<int, COMB_OPENORD> allOrd, myOrd;
	std::map<int, COMB_OPENORD>::iterator it0;
	std::vector<std::string> tickerList;	//tickerlist for request the latest bid/ask
	std::map<int, MODIFY_ORD> orderUpdate,tmpOrderUpdate;		//stock order for update
	std::map<std::string, QUOTE_DATA> quoteData;
	double minTick;
	double spreadBps;
	double bid;
	double ask;
	double myBps;
	int count = 0;

	myOrderIdList = orderIdList;

	allOrd = queryOrd();	//request all open orders

	//find the remaining open orders in orderIdList from all the request open order
	for (int i = 0; i < myOrderIdList.size(); i++) {
		it0 = allOrd.find(myOrderIdList[i]);
		if (it0 != allOrd.end()) {
			minTick = queryMinTick(it0->second.openOrd.ticker);
			myOrd.insert(*it0);
			tickerList.push_back(it0->second.openOrd.ticker);
			int remQty = it0->second.openOrd.action == "BUY" ? it0->second.ordStatus.remaining : -it0->second.ordStatus.remaining;	//if action="BUY", set quantity positive; otherwise negative
			orderUpdate[it0->first]={ it0->second.openOrd.ticker, -2, remQty, minTick };	//set the updateorder price to -2 for now
		}
	}

	if (myOrd.size() == 0) {
		std::cout << "Remain open order size is zero. Stop update." << std::endl;
		return;
	}

	do {
		count++;

		std::cout << "\n" << "Remain open order size: " << myOrd.size() << std::endl;

		for (std::map<int, COMB_OPENORD>::iterator it = myOrd.begin(); it != myOrd.end(); ++it) {
			std::cout << "Remain open order: \n"<<it->first << " => " << (it->second).openOrd.ticker << " action:" << (it->second).openOrd.action << " totalQty: " << (it->second).openOrd.totalQty
				<< ". Remaining: " << (it->second).ordStatus.remaining << ". ClientId: " << (it->second).ordStatus.clientId << "\n";
		}

		quoteData = queryQuote(tickerList);

		for (int i = 0; i < tickerList.size(); i++) {

			spreadBps = (quoteData[tickerList[i]].askPrice[0] - quoteData[tickerList[i]].bidPrice[0]) / quoteData[tickerList[i]].bidPrice[0] * 10000;
			std::cout << "ticker = " << tickerList[i] << ". minTick =" << orderUpdate[i].minTick << std::endl;
			//if spreadBps is less than the input Bps, use spreadBps (update price to bid/ask for SELL/BUY)
			myBps = spreadBps < aggBps ? spreadBps : aggBps;

			//orderQty>0: BUY, update price = bid price + aggbps; orderQty<0: SELL, update price = ask price - aggbps
			bid = roundNum(quoteData[tickerList[i]].bidPrice[0] * (1 + myBps / 10000), orderUpdate[i].minTick);	//round price to two digits after decimal and the minTick
			ask = roundNum(quoteData[tickerList[i]].askPrice[0] * (1 - myBps / 10000), orderUpdate[i].minTick);
			orderUpdate[i].orderPrice = orderUpdate[i].orderQty>0 ? bid :ask;

			std::cout << "ticker: " << orderUpdate[i].ticker << ". Spread bps = " << spreadBps << ". mybps = " << myBps << "Min tick = " << orderUpdate[i].minTick
				<< ". update price: " << orderUpdate[i].orderPrice << " quantity: " << orderUpdate[i].orderQty << std::endl;
		}

		//myOrderIdList = modifyLmtOrder(orderUpdate);

		Sleep(waitTime_s*1000);	//wait time for order to fill

		allOrd.clear();
		myOrd.clear();

		allOrd = queryOrd();

		//find the remaining open orders in orderIdList from all the request open order
		for (int i = 0; i < myOrderIdList.size(); i++) {
			it0 = allOrd.find(myOrderIdList[i]);
			if (it0 != allOrd.end()) {
				myOrd.insert(*it0);
				tickerList.push_back(it0->second.openOrd.ticker);
				int remQty = it0->second.openOrd.action == "BUY" ? it0->second.ordStatus.remaining : -it0->second.ordStatus.remaining;	//if action="BUY", set quantity positive; otherwise negative
				tmpOrderUpdate[it0->first] = { orderUpdate[it0->first].ticker, -2, remQty, orderUpdate[it0->first].minTick };
			}
			orderUpdate.clear();
			orderUpdate = tmpOrderUpdate;
			tmpOrderUpdate.clear();
		}

	} while (myOrd.size() > 0);

}

void IBAPI::closeAllStockAP(double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
	bool forceCompletion, bool allowPastTime, double monetaryValue) {

	printInfo("Close all positions using Arrival Price algo. ");

	std::vector<STOCK_ORD> closeOrd;
	STOCK_ORD tmpOrd;
	//std::vector<std::string> closeTickerList;
	bool zeroPos = true;	// make sure there is position to close.

	std::vector<POS> Position = queryPos();

	if (Position.size() == 0) {
		std::cout << "No position hold. Cannot close." << std::endl;
		return;
	}

	for (int i = 0; i < Position.size(); i++) {
		//closeTickerList.push_back(Position[i].ticker);
		if (Position[i].posQty != 0) {
			zeroPos = false;
		}
	}

	if (zeroPos == true) {
		std::cout << "No position hold. Cannot close. zeroPos = " << zeroPos << std::endl;
		return;
	}

	for (int i = 0; i < Position.size(); i++) {
		//only close stock positions for now
		if (Position[i].posQty != 0 && Position[i].secType=="STK" ) {
			// posQty>0: buy position. Need to sell at ask price to close; 
			// posQty<0: sell position. Need to buy at bid price to close.
			std::string primaryExchange = queryPriExch(Position[i].ticker);
			tmpOrd = { Position[i].ticker, -1, -Position[i].posQty, primaryExchange};	//set price to -1 because use market order
			std::cout << "Close ticker:" << tmpOrd.ticker << ". Close position: " << tmpOrd.orderQty << std::endl;
			closeOrd.push_back(tmpOrd);
		}
	}

	printInfo("Send Arrival Price close orders. ");

	sendAPOrder(closeOrd, maxPctVol, riskAversion, startTime, endTime, forceCompletion, allowPastTime, monetaryValue);
}

void IBAPI::closePartAP(std::vector<STOCK_ORD> orderList, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
	bool forceCompletion, bool allowPastTime, double monetaryValue) {
	printInfo("Close part of the positions using Arrival Price algo. ");

	std::vector<std::string> orderTicker;
	std::vector<STOCK_ORD> closeOrd;
	bool zeroPos = true;	// make sure there is position to close.

	std::vector<POS> Position = queryPos();

	if (Position.size() == 0) {
		std::cout << "No position hold. Cannot close." << std::endl;
		return;
	}

	for (int i = 0; i < orderList.size(); i++) {
		orderTicker.push_back(orderList[i].ticker);
	}
	
	//Exctract positions that are in the order ticker lists to generate close orders
	for (int i = 0; i < Position.size(); i++) {
		if (Position[i].posQty != 0 && std::find(orderTicker.begin(), orderTicker.end(), Position[i].ticker) != orderTicker.end()) {
			std::string primaryExchange = queryPriExch(Position[i].ticker);
			// posQty>0: buy position. Need to sell at ask price to close; 
			// posQty<0: sell position. Need to buy at bid price to close.
			STOCK_ORD tmpOrd = { Position[i].ticker, -1, -Position[i].posQty, primaryExchange };	//set price to -1 due to market order
			std::cout << "Close ticker:" << tmpOrd.ticker << ". Close position: " << tmpOrd.orderQty << std::endl;
			closeOrd.push_back(tmpOrd);
			zeroPos = false;
		}
			
	}
	
	if (zeroPos == true) {
		std::cout << "No position to close. Cannot close. zeroPos = " << zeroPos << std::endl;
		return;
	}

	printInfo("Send Arrival Price close orders. ");

	sendAPOrder(closeOrd, maxPctVol, riskAversion, startTime, endTime, forceCompletion, allowPastTime, monetaryValue);
}

void IBAPI::closePartVWAP(std::vector<STOCK_ORD> orderList, double maxPctVol, std::string startTime, std::string endTime,
	bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue) {
	printInfo("Close part of the positions using VWAP algo. ");

	std::vector<std::string> orderTicker;
	std::vector<STOCK_ORD> closeOrd;
	bool zeroPos = true;	// make sure there is position to close.

	std::vector<POS> Position = queryPos();

	if (Position.size() == 0) {
		std::cout << "No position hold. Cannot close." << std::endl;
		return;
	}

	for (int i = 0; i < orderList.size(); i++) {
		orderTicker.push_back(orderList[i].ticker);
	}

	//Exctract positions that are in the order ticker lists to generate close orders
	for (int i = 0; i < Position.size(); i++) {
		if (Position[i].posQty != 0 && std::find(orderTicker.begin(), orderTicker.end(), Position[i].ticker) != orderTicker.end()) {
			std::string primaryExchange = queryPriExch(Position[i].ticker);
			// posQty>0: buy position. Need to sell at ask price to close; 
			// posQty<0: sell position. Need to buy at bid price to close.
			STOCK_ORD tmpOrd = { Position[i].ticker, -1, -Position[i].posQty, primaryExchange };	//set price to -1 due to market order
			std::cout << "Close ticker:" << tmpOrd.ticker << ". Close position: " << tmpOrd.orderQty << "\n";
			closeOrd.push_back(tmpOrd);
			zeroPos = false;
		}

	}

	if (zeroPos == true) {
		std::cout << "No position to close. Cannot close. zeroPos = " << zeroPos << std::endl;
		return;
	}

	printInfo("Send VWAP close orders. ");

	sendVWAPOrder(closeOrd, maxPctVol, startTime, endTime, allowPastEndTime, noTakeLiq, speedUp, monetaryValue);
}

std::vector<int> IBAPI::openMktAP(std::vector<STOCK_ORD> stockOrd, double maxPctVol, std::string riskAversion, std::string startTime, std::string endTime,
	bool forceCompletion, bool allowPastTime, double monetaryValue) {

	printInfo("Submit orders using Arrival Price algo. ");

	for (int i = 0; i < stockOrd.size(); i++) {
		std::cout << "Ticker: " << stockOrd[i].ticker << ". Order quantity: " << stockOrd[i].orderQty << std::endl;
	}

	printInfo("Sending Arrival Price orders. ");

	return sendAPOrder(stockOrd, maxPctVol, riskAversion, startTime, endTime, forceCompletion, allowPastTime, monetaryValue);
}

std::vector<int> IBAPI::openMktVWAP(std::vector<STOCK_ORD> stockOrd, double maxPctVol, std::string startTime, std::string endTime,
	bool allowPastEndTime, bool noTakeLiq, bool speedUp, double monetaryValue) {

	printInfo("Submit orders using VWAP algo. ");

	for (int i = 0; i < stockOrd.size(); i++) {
		std::cout << "Ticker: " << stockOrd[i].ticker << ". Order quantity: " << stockOrd[i].orderQty << std::endl;
	}

	printInfo("Sending VWAP orders. ");

	return sendVWAPOrder(stockOrd, maxPctVol, startTime, endTime, allowPastEndTime, noTakeLiq, speedUp, monetaryValue);
}