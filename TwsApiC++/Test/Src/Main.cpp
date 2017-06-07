#include "IBAPI.h"

using namespace TwsApi;


#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>
#include <time.h> 

#define TEST( T, X ) ( printf( "T%7d %s\n", T, #X ), X )
#define CurrentThreadId GetCurrentThreadId
#define PrintProcessId printf("%ld  ", CurrentThreadId() )

int main(int argc, char *argv[])
{
	std::vector<std::string> lines;

	//std::cout << "argument numbers: " << argc << std::endl;
	if (argc !=2) {
		std::cerr << "Usage: " << argv[0] << " Config file path" << std::endl;
		return 1;
	}
	else {
		std::ifstream config_file(argv[1]);
		if (!config_file.is_open()) {
			std::cout << "Could not open file\n";
			return 1;
		}
		else {
			std::cout << "Open config file..." << std::endl;
			
			std::string tmpline;
			while (std::getline(config_file, tmpline)) {
				lines.push_back(tmpline);
			}
			config_file.close();
		}
	}

	int port = std::stoi(lines[0]);
	double multiplier = std::stod(lines[1]);
	std::string openStartTime = lines[2];
	std::string openEndTime = lines[3];
	std::string closeStartTime = lines[4];
	std::string closeEndTime = lines[5];

	std::cout << "Port = " << port << ". Multiplier = " << multiplier << "\n";
	std::cout << "Open time: " << openStartTime << "=>" << openEndTime << "\n";
	std::cout << "Close time: " << closeStartTime << "=>" << closeEndTime << "\n";

	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	std::string currentTime = asctime(timeinfo);

	std::cout << "Current time: " << currentTime << "string size =" << currentTime.size() << std::endl;

	printf("APIVersion    = %s\n", EClientL0::apiVersion());

	IBAPI testAPI;

	std::vector<std::string> tickerList = { "AAPL","PLCE","AMZN" };

	printf("ClientVersion = %d\n", testAPI.EC->clientVersion());

	std::cout << "nextValidId? = " << testAPI.EW.b_nextValidId << std::endl;

	TEST(0, testAPI.EC->eDisconnect());	// only for test purposes

	if (TEST(0, testAPI.EC->eConnect("", port, 100)))
	{
		PrintProcessId, printf("ServerVersion = %d\n", testAPI.EC->serverVersion());

		// pause the program until nextValidId is ready
		testAPI.waitForNextValidId();

		std::map<std::string, QUOTE_DATA> quoteMap;
		std::vector<STOCK_POS> stockPos;
		std::map<int, COMB_OPENORD> combOrd;
		/*
		quoteMap = testAPI.queryQuote(tickerList);

		for (int i = 0; i < tickerList.size(); i++) {
			std::cout << "Ticker: " << tickerList[i] << ". Ask: " << quoteMap[tickerList[i]].askPrice[0]
				<< ". Bid: " << quoteMap[tickerList[i]].bidPrice[0] << ". Ask size: " << quoteMap[tickerList[i]].askSize[0]
				<< ". Bid size: " << quoteMap[tickerList[i]].bidSize[0] << std::endl;
		}
		*/
		//int orderID = testAPI.queryNextOrderId();

		//Order baseOrder = OrderSamples::LimitOrder("BUY", 100, 1010);
		//Order baseOrder = OrderSamples::MarketOrder("BUY", 100);
		//testAPI.FillArrivalPriceParams(baseOrder, 0.1, "Passive", "09:53:00 PST", "10:00:00 PST", true, false, 100000);
		//testAPI.EC->placeOrder(orderID++, ContractSamples::USStock("AMZN"), baseOrder);

		//testAPI.closeAllAP(0.1, "Passive", "12:58:00 PST", "13:00:00 PST", true, false, 100000);

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

	

		std::vector<CSV_READ> CSVRead = testAPI.getCSV("D:\\Dropbox\\Public\\Finance\\UEI1001\\erst.UEI001.2017-06-08.csv");

		for (int i = 0; i < CSVRead.size(); i++) {
			std::cout << "Ticker: " << CSVRead[i].ticker << ". Score = " << CSVRead[i].score << ". Price: " << CSVRead[i].price << ". DMV: " << CSVRead[i].dmv << std::endl;
		}

		std::vector<STOCK_ORD> gOrder = testAPI.genOrder(CSVRead, multiplier);
		for (int i = 0; i < gOrder.size(); i++) {
			std::cout << "Ticker: " << gOrder[i].ticker << ". Price: " << gOrder[i].orderPrice << ". share: " << gOrder[i].orderQty << std::endl;
		}

		std::cout << "Continue program? (y/n)";
		char input;
		std::cin >> input;
		if (input == 'n' || input == 'N')
			return 1;
		else std::cin.ignore();

		
		std::vector<int> openOrderList = testAPI.openMktAP(gOrder, 0.05, "Passive", openStartTime, openEndTime, true, false, 100000);

		for (int i = 0; i < 30; i++) {
			Sleep(1000 * 60);	//sleep for 30mins
			
			combOrd = testAPI.queryOrd();

			std::cout << "Open order size =" << combOrd.size() << std::endl;

			for (std::map<int, COMB_OPENORD>::iterator it = combOrd.begin(); it != combOrd.end(); ++it) {
				std::cout << it->first << " => " << (it->second).openOrd.ticker << " action:" << (it->second).openOrd.action << " totalQty: " << (it->second).openOrd.totalQty
					<< ". Remaining: " << (it->second).ordStatus.remaining << ". ClientId: " << (it->second).ordStatus.clientId << "\n";

			}
		}

		std::cout << "Enter to continue program";
		std::cin.ignore();

		testAPI.closeAllAP( 0.05, "Passive", closeStartTime, closeEndTime, true, false, 100000);
		

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
		

		//std::vector<int> tmpList = testAPI.openMktAP(tstockOrd, 0.05, "Passive", "06:30:00 PST", "06:45:00 PST", true, false, 100000);

		//testAPI.closeAllAP(0.05, "Passive", "11:21:00 EST", "11:32:00 EST", true, false, 100000);

		//double t = 392.6;
		//std::cout << int(t/100)*100 << std::endl;

		//testAPI.EC->reqGlobalCancel();
		
	}
	
	TEST(0, testAPI.EC->eDisconnect());

	{ PrintProcessId, printf("Press return to end\n"); char s[10]; gets_s(s); }
	return 0;
}
