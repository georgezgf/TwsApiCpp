#include "IBAPI.h"

using namespace TwsApi;


#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>


#define TEST( T, X ) ( printf( "T%7d %s\n", T, #X ), X )
#define CurrentThreadId GetCurrentThreadId
#define PrintProcessId printf("%ld  ", CurrentThreadId() )

int main(void)
{
	printf("APIVersion    = %s\n", EClientL0::apiVersion());

	IBAPI testAPI;

	std::vector<std::string> tickerList = { "AAPL","FB","AMZN" };

	printf("ClientVersion = %d\n", testAPI.EC->clientVersion());

	std::cout << "nextValidId? = " << testAPI.EW.b_nextValidId << std::endl;

	TEST(0, testAPI.EC->eDisconnect());	// only for test purposes

	if (TEST(0, testAPI.EC->eConnect("", 7497, 100)))
	{
		PrintProcessId, printf("ServerVersion = %d\n", testAPI.EC->serverVersion());

		// pause the program until nextValidId is ready
		testAPI.EW.waitForNextValidId();

		std::map<std::string, QUOTE_DATA> quoteMap;
		std::vector<STOCK_POS> stockPos;
		std::vector<OPEN_ORD> open_Ord;
		/*
		quoteMap = testAPI.queryQuote(tickerList);

		for (int i = 0; i < tickerList.size(); i++) {
			std::cout << "Ticker: " << tickerList[i] << ". Ask: " << quoteMap[tickerList[i]].askPrice[0]
				<< ". Bid: " << quoteMap[tickerList[i]].bidPrice[0] << ". Ask size: " << quoteMap[tickerList[i]].askSize[0]
				<< ". Bid size: " << quoteMap[tickerList[i]].bidSize[0] << std::endl;
		}
		
		*/
		std::vector<STOCK_ORD> testOrder = { {"TWTR",124,100},{"FB",40,-200},{"AMZN",1000,100} };
		
		std::vector<int> orderIdList = testAPI.sendLmtOrder(testOrder);		

		for (int i = 0; i < orderIdList.size(); i++) {
			std::cout << "orderId = " << orderIdList[i] << std::endl;
		}
		
		stockPos = testAPI.queryPos();


		std::cout << "Position size =" << stockPos.size() << std::endl;

		for (int i = 0; i < stockPos.size(); i++) {
			std::cout << "Ticker: " << stockPos[i].ticker << ". Position: " << stockPos[i].posQty << ". Avg cost: " << stockPos[i].avgCost << std::endl;
		}
		/*
		open_Ord = testAPI.queryOrd();

		std::cout << "Open order size =" << open_Ord.size() << std::endl;

		for (int i = 0; i <open_Ord.size(); i++) {
			std::cout << "Ticker: " << open_Ord[i].ticker << ". Quantity: " << open_Ord[i].totalQty 
				<< ". Action: " << open_Ord[i].action << ". OrderId = "<< open_Ord[i].OrderId <<std::endl;
		}
		*/
		int cash = testAPI.queryCash();

		std::cout << "Cash = " << cash << std::endl;
	
		/*
		std::vector<STOCK_ORD> orderCSV = testAPI.getCSV("D:\\Dropbox\\Public\\Finance\\sendOrd_test.csv");


		for (int i = 0; i < orderCSV.size(); i++) {
			std::cout << "Ticker: " <<  orderCSV[i].ticker <<  ". Price: "<< orderCSV[i].orderPrice << ". Qty: "<< orderCSV[i].orderQty<<std::endl;
		}
		*/
		testAPI.EC->reqGlobalCancel();

		//testAPI.sendLmtOrder(orderCSV);

		//std::vector<std::string> results = testAPI.getCSV(file);

		//std::cout << "csv string: " << results.size() << std::endl;


		//testAPI.closeAllPos();
		//testAPI.openMktLmt(testOrder);
		
	}
	
	TEST(0, testAPI.EC->eDisconnect());

	{ PrintProcessId, printf("Press return to end\n"); char s[10]; gets_s(s); }
	return 0;
}
