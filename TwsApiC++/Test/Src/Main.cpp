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

	std::vector<std::string> tickerList = { "AAPL","PLCE","AMZN" };

	printf("ClientVersion = %d\n", testAPI.EC->clientVersion());

	std::cout << "nextValidId? = " << testAPI.EW.b_nextValidId << std::endl;

	TEST(0, testAPI.EC->eDisconnect());	// only for test purposes

	if (TEST(0, testAPI.EC->eConnect("", 7497, 100)))
	{
		PrintProcessId, printf("ServerVersion = %d\n", testAPI.EC->serverVersion());

		// pause the program until nextValidId is ready
		testAPI.waitForNextValidId();

		std::map<std::string, QUOTE_DATA> quoteMap;
		std::vector<STOCK_POS> stockPos;
		std::map<int, COMB_OPENORD> combOrd;
		
		quoteMap = testAPI.queryQuote(tickerList);

		for (int i = 0; i < tickerList.size(); i++) {
			std::cout << "Ticker: " << tickerList[i] << ". Ask: " << quoteMap[tickerList[i]].askPrice[0]
				<< ". Bid: " << quoteMap[tickerList[i]].bidPrice[0] << ". Ask size: " << quoteMap[tickerList[i]].askSize[0]
				<< ". Bid size: " << quoteMap[tickerList[i]].bidSize[0] << std::endl;
		}
		
		int orderID = testAPI.queryNextOrderId();

		//Order baseOrder = OrderSamples::LimitOrder("BUY", 100, 1010);
		//Order baseOrder = OrderSamples::MarketOrder("BUY", 100);
		//testAPI.FillArrivalPriceParams(baseOrder, 0.1, "Passive", "09:53:00 PST", "10:00:00 PST", true, false, 100000);
		//testAPI.EC->placeOrder(orderID++, ContractSamples::USStock("AMZN"), baseOrder);

		testAPI.closeAllAP(0.1, "Passive", "12:58:00 PST", "13:00:00 PST", true, false, 100000);

		/*
		std::vector<STOCK_ORD> testOrder = { {"TGI",1.8,100},{"PLCE",1.5,-200} };
		
		std::vector<int> orderIdList = testAPI.sendLmtOrder(testOrder);
		
		for (int i = 0; i < orderIdList.size(); i++) {
			std::cout << "orderId = " << orderIdList[i] << std::endl;
		}
		
		*/

		/*
		stockPos = testAPI.queryPos();

		std::cout << "Position size =" << stockPos.size() << std::endl;

		for (int i = 0; i < stockPos.size(); i++) {
			std::cout << "Ticker: " << stockPos[i].ticker << ". Position: " << stockPos[i].posQty << ". Avg cost: " << stockPos[i].avgCost << std::endl;
		}
		*/

		//testAPI.updateOrder({ 165,166 },2,5);
		//std::cout << testAPI.queryMinTick("TGI") << std::endl;

		//testAPI.EC->reqContractDetails(9002, ContractSamples::USStock("PLCE"));

		/*
		for (int i = 0; i < tickerList.size(); i++) {
			double minTick = testAPI.queryMinTick(tickerList[i]);
			std::cout << "ticker: " << tickerList[i] << ". min Tick = " << minTick << std::endl;
		}
		*/
		//std::cout << testAPI.roundNum(147.65222, 0.01) << std::endl;
		//std::cout << 148 % 5 << "  " << 148/5<<std::endl;

		/*
		combOrd = testAPI.queryOrd();

		std::cout << "Open order size =" << combOrd.size() << std::endl;

		for (std::map<int, COMB_OPENORD>::iterator it = combOrd.begin(); it != combOrd.end(); ++it) {
			std::cout << it->first << " => " << (it->second).openOrd.ticker <<" action:"<<  (it->second).openOrd.action << " totalQty: "<< (it->second).openOrd.totalQty
				<<". Remaining: " << (it->second).ordStatus.remaining << ". ClientId: " << (it->second).ordStatus.clientId <<"\n";
			
		}
		*/

		/*
		for (int i = 0; i <open_Ord.size(); i++) {
			std::cout << "Ticker: " << open_Ord[i].ticker << ". Quantity: " << open_Ord[i].totalQty 
				<< ". Action: " << open_Ord[i].action << ". OrderId = "<< open_Ord[i].OrderId <<std::endl;
		}
		*/


		/*
		int cash = testAPI.queryCash();
		std::cout << "Cash = " << cash << std::endl;
	*/
		/*
		std::vector<STOCK_ORD> orderCSV = testAPI.getCSV("D:\\Dropbox\\Public\\Finance\\sendOrd_test.csv");

		for (int i = 0; i < orderCSV.size(); i++) {
			std::cout << "Ticker: " <<  orderCSV[i].ticker <<  ". Price: "<< orderCSV[i].orderPrice << ". Qty: "<< orderCSV[i].orderQty<<std::endl;
		}
		*/

		//testAPI.EC->reqGlobalCancel();
		
	}
	
	TEST(0, testAPI.EC->eDisconnect());

	{ PrintProcessId, printf("Press return to end\n"); char s[10]; gets_s(s); }
	return 0;
}
