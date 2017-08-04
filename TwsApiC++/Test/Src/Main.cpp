#include "IBAPI.h"

using namespace TwsApi;


#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>
#include <time.h> 

#define TEST( T, X ) ( printf( "T%7d %s\n", T, #X ), X )
#define CurrentThreadId GetCurrentThreadId
#define PrintProcessId printf("%ld  ", CurrentThreadId() )

#define READCSV		//uncomment this to read CSV files
#define SUBMITORDER	//uncomment this to submit open and close orders
//#define TESTFUN		//uncomment this to test functions

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
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	//std::string currentTime = asctime(timeinfo);
	strftime(buffer, 80, "%F", timeinfo);
	std::string buffers(buffer);

	//std::cout << "Current time: " << currentTime << "string size =" << currentTime.size() << std::endl;
	//std::cout << "time = " << buffer << ".  " << buffers <<std::endl;
	//std::cout << "D:\\Dropbox\\Public\\Finance\\UEI1001\\erst.UEI001." + buffers << std::endl;

	printf("APIVersion    = %s\n", EClientL0::apiVersion());

	IBAPI testAPI;

	std::vector<std::string> tickerList = { "AAPL","PLCE","AMZN" };

	printf("ClientVersion = %d\n", testAPI.EC->clientVersion());

	std::cout << "nextValidId? = " << testAPI.EW.b_nextValidId << std::endl;

	TEST(0, testAPI.EC->eDisconnect());	// only for test purposes

	if (TEST(0, testAPI.EC->eConnect("", port, 50)))
	{
		PrintProcessId, printf("ServerVersion = %d\n", testAPI.EC->serverVersion());

		// pause the program until nextValidId is ready
		testAPI.waitForNextValidId();

		std::map<std::string, QUOTE_DATA> quoteMap;
		std::vector<POS> allPos;
		std::map<int, COMB_OPENORD> combOrd;

		
#ifdef READCSV
	
		std::string csvFile = "D:\\Dropbox\\Public\\Finance\\syncFile\\UEI001\\erst.UEI001." + buffers + ".csv";
		//std::string csvFile = "D:\\Dropbox\\Public\\Finance\\syncFile\\UEI001\\erst.UEI001.2017-07-28.csv";


		std::vector<CSV_READ> CSVRead = testAPI.getCSV(csvFile);

		if (CSVRead.size() == 0) {
			std::cout << "Read csv failed. Stop program" << std::endl;
			return 1;
		}

		for (int i = 0; i < CSVRead.size(); i++) {
			std::cout << "Ticker: " << CSVRead[i].ticker << " Score = " << CSVRead[i].score << ". limit Price: " << CSVRead[i].lmtPrice << ". DMV: " << CSVRead[i].dmv << "\n";
		}
		
		double bp = testAPI.queryBuyingPower();
		std::cout << "Buying power = " << bp << std::endl;
		
		std::vector<STOCK_ORD> gOrder = testAPI.genOrder(CSVRead, multiplier,bp);

		if (gOrder.size() == 0) {
			std::cout << "There is no stock to trade today. Stop program" << std::endl;
			return 1;
		}
		
		for (int i = 0; i < gOrder.size(); i++) {
			double minTick = testAPI.queryMinTick(gOrder[i].ticker);

			std::cout << "Ticker: " << gOrder[i].ticker << ". Primary exchange: " << gOrder[i].primaryExch << ". Price: " << gOrder[i].orderPrice 
				<< ". share: " << gOrder[i].orderQty << ". minTick = " << minTick <<". Trunc limit price: " << testAPI.roundNum(gOrder[i].orderQty,gOrder[i].orderPrice,minTick)<<"\n";

			gOrder[i].orderPrice = testAPI.roundNum(gOrder[i].orderQty, gOrder[i].orderPrice, minTick);
		}
		
		std::vector<double> exposure = testAPI.orderHedgeCal(gOrder);
		
		std::vector<STOCK_ORD> hedgeOrder = testAPI.genHedgeOrder(exposure[0], exposure[1]);

		//combine stock orders and hedge orders
		std::vector<STOCK_ORD> totalOrder;
		totalOrder.reserve(gOrder.size() + hedgeOrder.size());
		totalOrder.insert(totalOrder.end(), gOrder.begin(), gOrder.end());
		totalOrder.insert(totalOrder.end(), hedgeOrder.begin(), hedgeOrder.end());
		
	

		/*
		for (int i = 0; i < 100; i++) {
			testAPI.monitorExp(gOrder);
			Sleep(1000 * 60);
		}
		*/
#endif
		/*
		std::cout << "Continue program? (y/n)";
		char input;
		std::cin >> input;
		if (input == 'n' || input == 'N')
			return 1;
		else std::cin.ignore();
		*/
		//testAPI.EC->reqFundamentalData(8001, ContractSamples::USStock("ADI"), "CalendarReport");
		//Sleep(2000);
		//testAPI.EC->cancelFundamentalData(8001);

#ifdef SUBMITORDER
		
		//If everything is OK, submit open market arrival price orders
		//std::vector<int> openOrderList = testAPI.openMktAP(gOrder, 0.05, "Passive", openStartTime, openEndTime, false, false, 100000);
		//std::vector<int> hedgeOrderList = testAPI.openMktAP(hedgeOrder, 0.05, "Passive", openStartTime, openEndTime, false, true, 100000);
		//testAPI.EC->reqGlobalCancel();
		
		

		if (port == 7500) {	//use limit on open order for lydzz account
			std::vector<int> openOrderList = testAPI.sendLOOOrder(gOrder);
		}
		else {	//all the other accounts use VWAP
			std::vector<int> openOrderList = testAPI.openMktVWAP(gOrder, 0.05, openStartTime, openEndTime, false, false, false, 100000);
		}
		

		//std::vector<int> hedgeOrderList = testAPI.openMktVWAP(hedgeOrder, 0.05, closeStartTime, closeEndTime, false, false, true, 100000);
		
		Sleep(3000 * 60);

		//Every 1 min, read open orders, total 40 mins
		for (int i = 0; i < 40; i++) {
			
			combOrd = testAPI.queryOrd();

			std::cout << "Open order size =" << combOrd.size() << std::endl;

//			for (std::map<int, COMB_OPENORD>::iterator it = combOrd.begin(); it != combOrd.end(); ++it) {
//				std::cout << it->first << " => " << (it->second).openOrd.ticker << " action:" << (it->second).openOrd.action << " totalQty: " << (it->second).openOrd.totalQty
//					<< ". Remaining: " << (it->second).ordStatus.remaining << ". ClientId: " << (it->second).ordStatus.clientId << "\n";
//			}

//			testAPI.monitorExp(gOrder);

			if (combOrd.size() == 0) {
				allPos = testAPI.queryPos(); 

				std::cout << "Position size =" << allPos.size() << std::endl;

				for (int i = 0; i < allPos.size(); i++) {
					std::cout << "Ticker: " << allPos[i].ticker << ". Security type: " << allPos[i].secType << ". Position: " 
						<< allPos[i].posQty << ". Avg cost: " << allPos[i].avgCost << "\n";
				}

				break;
			}

			Sleep(1000 * 60);	//sleep for 1 min
		}
		
		

		/*
		std::cout << "Enter to continue program";
		std::cin.ignore();
		//*/

		//testAPI.EC->reqGlobalCancel();

		//Sleep(5000);

		testAPI.closePartVWAP(totalOrder, 0.05, closeStartTime, closeEndTime, true, false, false, 100000);
		//testAPI.closePartAP(gOrder, 0.05, "Passive", closeStartTime, closeEndTime, true, false, 100000);
		//testAPI.closeAllStockAP( 0.05, "Passive", closeStartTime, closeEndTime, true, false, 100000);
		
#endif

#ifdef TESTFUN
		double cash = testAPI.queryCash();
		std::cout << "Cash = " << cash << std::endl;
		
//		std::cout << testAPI.roundNum(26.73, 0.0001) << std::endl;

//		double bp = testAPI.queryBuyingPower();
//		std::cout << "Buying power = " << bp << std::endl;

	
		//testAPI.EC->reqMktData(100, ContractSamples::USStock("SPY"), "", false);
		//Sleep(5000);
#endif


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
