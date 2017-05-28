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

		//testAPI.sendLmtOrder(testOrder);

		//quoteMap = testAPI.queryQuote(tickerList);

		stockPos = testAPI.queryPos();

		std::cout << "Position size =" << stockPos.size() << std::endl;

		for (int i = 0; i < stockPos.size(); i++) {
			std::cout << "Ticker: " << stockPos[i].ticker << ". Position: " << stockPos[i].posQty << ". Avg cost: " << stockPos[i].avgCost << std::endl;
		}

		open_Ord = testAPI.queryOrd();

		std::cout << "Open order size =" << open_Ord.size() << std::endl;

		for (int i = 0; i <open_Ord.size(); i++) {
			std::cout << "Ticker: " << open_Ord[i].ticker << ". Quantity: " << open_Ord[i].totalQty 
				<< ". Action: " << open_Ord[i].action << ". OrderId = "<< open_Ord[i].OrderId <<std::endl;
		}

		int cash = testAPI.queryCash();

		std::cout << "Cash = " << cash << std::endl;
		/*
		std::ifstream file("D:\\Dropbox\\Public\\Finance\\sendOrd_test.csv");
		
		for (CSVIterator loop(file); loop != CSVIterator(); ++loop)
		{
			std::cout << "4th Element(" << (*loop)[3] << ")\n";
		}
		*/
		
		std::vector<STOCK_ORD> orderCSV = testAPI.getCSV("D:\\Dropbox\\Public\\Finance\\sendOrd_test.csv");


		for (int i = 0; i < orderCSV.size(); i++) {
			std::cout << "Ticker: " <<  orderCSV[i].ticker <<  ". Price: "<< orderCSV[i].orderPrice << ". Qty: "<< orderCSV[i].orderQty<<std::endl;
		}
		
		//std::vector<std::string> results = testAPI.getCSV(file);

		//std::cout << "csv string: " << results.size() << std::endl;


		//testAPI.closeAllPos();
		//testAPI.openMktLmt(testOrder);

		//TEST(41, testAPI.EC->reqPositions());


		//TEST(55, testAPI.EC->placeOrder(MW.m_orderId++, ContractSamples::USStock("NVDA"), OrderSamples::LimitOrder("BUY", 300, 400)));
		//TEST(42, testAPI.EC->reqPositions());

		//TEST(11, testAPI.EC->reqGlobalCancel());

		//TEST(41, testAPI.EC->reqGlobalCancel());

		//TEST(42, testAPI.EC->reqOpenOrders());

		
	}

	int id = 1;
	while (id++ < 100)
	{
		//	if( !MW.IsCalledFromThread() ) testAPI.EC->checkMessages();
		Sleep(100);

		/* for 'stress' testing */
		//if (0 == id % 50)
		//TEST(id, testAPI.EC->reqMktData( id, C, "", false ) );
		/**/
		//if (30 == id)
		//TEST(id, testAPI.EC->eDisconnect());
		/**/
		//if (40 == id)
		//TEST(id, testAPI.EC->eDisconnect());


		//if (60 == id || 70 == id)
		//TEST(id, testAPI.EC->reqMktData(id, ContractSamples::USStock("AAPL"), "", false));	// will fail of course


		//if (130 == id)
		//TEST(id, testAPI.EC->eConnect("", 7497, 10));

		//if (150 == id)
		//TEST(id, testAPI.EC->eConnect("", 7497, 10));		// raises already connected error

		if (190 == id)
			TEST(id,
				testAPI.EC->reqHistoricalData
				(id
					, ContractSamples::USStock("NVDA")
					, EndDateTime(2015, 02, 20), DurationStr(1, *DurationHorizon::Days), *BarSizeSetting::_1_hour
					, *WhatToShow::TRADES
					, true
					, FormatDate::AsDate
				)
			);

		//if (200 == id)
		//TEST(id, testAPI.EC->reqMktData(id, ContractSamples::USStock("BABA"), "", false));

		/**/
		//if (800 == id)
		//TEST(id, testAPI.EC->cancelMktData(200));
		/**/
	}


	TEST(0, testAPI.EC->eDisconnect());

	{ PrintProcessId, printf("Press return to end\n"); char s[10]; gets_s(s); }
	return 0;
}
