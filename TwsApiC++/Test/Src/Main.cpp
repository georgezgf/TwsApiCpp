#include "IBAPI.h"

using namespace TwsApi;

#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>
#include <time.h> 
#include <iostream>
#include <fstream>

#define TEST( T, X ) ( printf( "T%7d %s\n", T, #X ), X )
#define CurrentThreadId GetCurrentThreadId
#define PrintProcessId printf("%ld  ", CurrentThreadId() )

#define READCSV		//uncomment this to read CSV files
#define SUBMITORDER	//uncomment this to submit open and close orders
//#define TESTFUN		//uncomment this to test functions
//#define BASKET
//#define ANDV
//#define ACCSUMMARY
//#define POSMONITOR
//#define HEDGEORDER

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
	double LOOdmvPercentage = std::stod(lines[2]);
	std::string openStartTime = lines[3];
	std::string openEndTime = lines[4];
	std::string closeStartTime = lines[5];
	std::string closeEndTime = lines[6];
	std::string erstFilePath = lines[7];
	std::string andvTradeFilePath = lines[8];
	std::string posFileSavePath = lines[9];
	std::string hedgeFileSavePath = lines[10];
	std::string accSummarySavePath = lines[11];

	std::cout << "Port = " << port << ". Multiplier = " << multiplier << "\n";
	std::cout << "LOO dmv Percentage = " << LOOdmvPercentage << ". LOO dmv Percentage = " << LOOdmvPercentage << "\n";
	std::cout << "Open time: " << openStartTime << "=>" << openEndTime << "\n";
	std::cout << "Close time: " << closeStartTime << "=>" << closeEndTime << "\n";
	
	std::cout << "Erst file path: " << erstFilePath << "\n";
	std::cout << "ANDV file path: " << andvTradeFilePath << "\n";
	std::cout << "Will save position files to: " << posFileSavePath << "\n";
	std::cout << "Will save hedge files to: " << hedgeFileSavePath << "\n";
	std::cout << "Will save account summary file to: " << accSummarySavePath << "\n";

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

	printf("ClientVersion = %d\n", testAPI.EC->clientVersion());

	std::cout << "nextValidId? = " << testAPI.EW.b_nextValidId << std::endl;

	TEST(0, testAPI.EC->eDisconnect());	// only for test purposes

	if (TEST(0, testAPI.EC->eConnect("", port, 3)))
	{
		PrintProcessId, printf("ServerVersion = %d\n", testAPI.EC->serverVersion());

		// pause the program until nextValidId is ready
		testAPI.waitForNextValidId();

		std::map<std::string, QUOTE_DATA> quoteMap;
		std::vector<POS> allPos;
		std::map<int, COMB_OPENORD> combOrd;

		
#ifdef READCSV
	
		//std::string csvFile = "D:\\Dropbox\\Public\\Finance\\syncFile\\UEI001\\erst.UEI001." + buffers + ".csv";
		//std::string csvFile = "D:\\Dropbox\\Public\\Finance\\syncFile\\UEI001\\erst.UEI001.2017-11-02.csv";
		std::string csvFile = erstFilePath + "erst.tradeData." + buffers + ".csv";
		//csvFile = erstFilePath + "erst.UEI001.2019-01-24.csv";

		std::vector<CSV_READ> CSVRead = testAPI.getCSV(csvFile);

		if (CSVRead.size() == 0) {
			std::cout << "Read csv failed. Stop program" << std::endl;
			return 1;
		}

		for (int i = 0; i < CSVRead.size(); i++) {
			std::cout << "Ticker: " << CSVRead[i].ticker <<  ". limit Price: " << CSVRead[i].lmtPrice << ". Order: " << CSVRead[i].qty << "\n";
		}
		
		double bp = testAPI.queryBuyingPower();
		std::cout << "Buying power = " << bp << std::endl;
		
		std::vector<STOCK_ORD> erstOrder = testAPI.genTradeOrder(CSVRead);
		//std::vector<STOCK_ORD> lyOrder = testAPI.genOrder(CSVRead, multiplier, LOOdmvPercentage, bp); //maxPercentage = 0.05% for ly (LOO orders)
		//std::vector<STOCK_ORD> ltOrder = testAPI.genOrder(CSVRead, 2, 0.002, 3000000);	//maxPercentage = 0.2% for lt (for both LOO and VWAP orders)
		

		erstOrder = testAPI.truncLmtPrice(erstOrder, FALSE); //truncate the price by minTick (because use LOO for open orders, need to fill limit price)

		if (erstOrder.size() == 0) {
			std::cout << "There is no stock to trade today. Stop program" << std::endl;
			return 1;
		}

		std::cout << "erstOrder size:" << erstOrder.size() << "\n";
		for (int i = 0; i < erstOrder.size(); i++) {

			std::cout << "Ticker: " << erstOrder[i].ticker << ". Primary exchange: " << erstOrder[i].primaryExch
				<< ". share: " << erstOrder[i].orderQty << ". Trunc limit price: " << erstOrder[i].orderPrice << "\n";
	
			//gOrder[i].orderPrice = testAPI.roundNum(gOrder[i].orderQty, gOrder[i].orderPrice, minTick);
		}
		
		
		//if (port == 7500) {	//ly
		//	//std::cout << "Order submission size: " << lyOrder.size() << std::endl;

		//	lyOrder = testAPI.truncLmtPrice(lyOrder,FALSE);//truncate the price by minTick (because use LOO for open orders, need to fill limit price)

		//	if (lyOrder.size() == 0) {
		//		std::cout << "There is no stock to trade today. Stop program" << std::endl;
		//		return 1;
		//	}

		//	std::cout << "lyOrder size:" << lyOrder.size() << "\n";
		//	for (int i = 0; i < lyOrder.size(); i++) {

		//		std::cout << "Ticker: " << lyOrder[i].ticker << ". Primary exchange: " << lyOrder[i].primaryExch
		//			<< ". share: " << lyOrder[i].orderQty << ". Trunc limit price: " << lyOrder[i].orderPrice << "\n";

		//		//gOrder[i].orderPrice = testAPI.roundNum(gOrder[i].orderQty, gOrder[i].orderPrice, minTick);
		//	}
		//}
		//if (port == 7498) {	//lt

		//	
		//	if (ltOrder.size() == 0) {
		//		std::cout << "There is no stock to trade today. Stop program" << std::endl;
		//		return 1;
		//	}

		//	ltOrder = testAPI.truncLmtPrice(ltOrder,FALSE);

		//	std::cout << "ltOrder size:" << ltOrder.size() << "\n";
		//	for (int i = 0; i < ltOrder.size(); i++) {

		//		std::cout << "Ticker: " << ltOrder[i].ticker << ". Primary exchange: " << ltOrder[i].primaryExch
		//			<< ". share: " << ltOrder[i].orderQty << ". Trunc limit price: " << ltOrder[i].orderPrice << "\n";

		//		//gOrder[i].orderPrice = testAPI.roundNum(gOrder[i].orderQty, gOrder[i].orderPrice, minTick);
		//	}
		//}
		/*
		std::vector<double> exposure = testAPI.orderHedgeCal(lyOrder);
		
		std::vector<STOCK_ORD> hedgeOrder = testAPI.genHedgeOrder(exposure[0], exposure[1]);

		//combine stock orders and hedge orders
		std::vector<STOCK_ORD> totalOrder;
		totalOrder.reserve(ltOrder.size() + hedgeOrder.size());
		totalOrder.insert(totalOrder.end(), ltOrder.begin(), lyOrder.end());
		totalOrder.insert(totalOrder.end(), hedgeOrder.begin(), hedgeOrder.end());
		*/
	

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

		std::vector<int> openOrderList = testAPI.sendLOOOrder(erstOrder);

		//if (port == 7500) {	//use limit on open order for ly
		//	std::vector<int> openOrderList = testAPI.sendLOOOrder(lyOrder);
		//}
		//if (port==7498) {	//all the other accounts use VWAP
		//	testAPI.splitLOOVWAPOrders(lyOrder, ltOrder,0.0005, 0.05, openStartTime, openEndTime, false, false, false, 100000);

		////	std::vector<int> openOrderList = testAPI.openMktVWAP(gOrder, 0.05, openStartTime, openEndTime, false, false, false, 100000);
		//}
		

		//std::vector<int> hedgeOrderList = testAPI.openMktVWAP(hedgeOrder, 0.05, closeStartTime, closeEndTime, false, false, true, 100000);
		
		Sleep(1000 * 60 * 20);

		//Every 1 min, read open orders, total 40 mins
		for (int i = 0; i < 45; i++) {

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

	//				for (int i = 0; i < allPos.size(); i++) {
	//					std::cout << "Ticker: " << allPos[i].ticker << ". Security type: " << allPos[i].secType << ". Position: " 
	//						<< allPos[i].posQty << ". Avg cost: " << allPos[i].avgCost << "\n";
	//				}
				break;
			}

			Sleep(1000 * 60);	//sleep for 1 min
		}

		testAPI.closePartVWAP(erstOrder, 0.05, closeStartTime, closeEndTime, true, false, false, 100000);


		/*
		std::cout << "Enter to continue program";
		std::cin.ignore();
		//*/

		//testAPI.EC->reqGlobalCancel();

		//Sleep(5000);
		//if (port == 7500) {
		//	testAPI.closePartVWAP(lyOrder, 0.05, closeStartTime, closeEndTime, true, false, false, 100000);
		//}

		//if (port == 7498) {
		//	testAPI.closePartVWAP(ltOrder, 0.05, closeStartTime, closeEndTime, true, false, false, 100000);
		//}

		//testAPI.closePartAP(gOrder, 0.05, "Passive", closeStartTime, closeEndTime, true, false, 100000);
		//testAPI.closeAllStockAP( 0.05, "Passive", closeStartTime, closeEndTime, true, false, 100000);

#endif

#ifdef TESTFUN

		//testAPI.closePartVWAP(lyOrder, 0.05, closeStartTime, closeEndTime, true, false, false, 100000);
		//testAPI.closeAllStockVWAP(0.05, closeStartTime, closeEndTime, true, false, false, 100000);
		//double cash = testAPI.queryCash();
		//std::cout << "Cash = " << cash << std::endl;

		//testAPI.sendFutureMktOrder("ESZ7", "GLOBEX", -1);	//E-mini S&P 500
		//testAPI.sendFutureMktOrder("TFZ7", "NYBOT", 1);	//Russel 2000 mini futures

		//std::vector<POS> futurePos = testAPI.queryPos();
		//for (int i = 0; i < futurePos.size(); i++) {
		//	std::cout << futurePos[i].ticker << ": " << futurePos[i].secType << ", " << futurePos[i].posQty << ".\n";
		//}

//		std::cout << testAPI.roundNum(26.73, 0.0001) << std::endl;

//		double bp = testAPI.queryBuyingPower();
//		std::cout << "Buying power = " << bp << std::endl;


		//testAPI.EC->reqMktData(100, ContractSamples::USStock("SPY"), "", false);
		//Sleep(5000);

		//read all positions and save them to csv file
	



#endif

#ifdef POSMONITOR
		//std::string anPosFile = "D:\\Cornerstone\\positions\\pos-" + buffers + ".csv";
		//std::string PosFile = "D:\\Cornerstone\\positions\\paper-pos-" + buffers + ".csv";
		std::string PosFile = posFileSavePath + "pos-" + buffers + ".csv";

		allPos = testAPI.queryPos();
		std::cout << "Position size =" << allPos.size() << "\n";

		while (TRUE) {
			std::ofstream outFile(PosFile);

			allPos = testAPI.queryPos();
			std::cout << "Position size =" << allPos.size() << "\n";

			outFile << "ticker, pos, cost, secType, localSymbol \n";

			for (int i = 0; i < allPos.size(); i++) {
				outFile << allPos[i].ticker + "," + std::to_string(allPos[i].posQty) + "," + std::to_string(allPos[i].avgCost) + "," + allPos[i].secType + "," + allPos[i].localSymbol + "\n";
			}

			outFile << "FILEEND";

			std::cout << "Finish writing position to file. Path: " << PosFile << "\n";
			outFile.close();

			Sleep(1000 * 60);
		}

#endif // POSMONITOR

#ifdef BASKET
		std::string anCSVFile = "D:\\Dropbox\\Public\\Finance\\syncFile\\anCSV\\AN.signal." + buffers + ".csv";
		std::vector<CSV_READ> anCSVRead = testAPI.getanCSV(anCSVFile);

		allPos = testAPI.queryPos();

		std::vector<STOCK_ORD> anOrder = testAPI.genANOrder(anCSVRead, allPos);


		if (port == 7500) {	
	
			if (anOrder.size() == 0) {
				std::cout << "There is no stock to trade today for AN event. Stop program" << std::endl;
				return 1;
			}

			std::cout << "anOrder size:" << anOrder.size() << "\n";
			for (int i = 0; i < anOrder.size(); i++) {

				std::cout << "Ticker: " << anOrder[i].ticker << ". Primary exchange: " << anOrder[i].primaryExch
					<< ". share: " << anOrder[i].orderQty << "\n";
			}
		}

		std::string basketFile = "D:\\Cornerstone\\Basket-" + buffers + ".csv";

		std::ofstream outFile(basketFile);

		strftime(buffer, 80, "%Y%m%d", timeinfo);
		std::string today(buffer);

		outFile << "Action,Quantity,Symbol,SecType,Exchange,Currency,TimeInForce,OrderType,BasketTag,Account,OrderRef,Algo noTakeLiq,Algo allowPastEndTime,Algo speedUp,Algo startTime,Algo maxPctVol,Algo strategy\n";

		for (int i = 0; i < anOrder.size(); i++) {
			std::string action = anOrder[i].orderQty > 0 ? "BUY" : "SELL";
			outFile << action + "," + std::to_string(abs(anOrder[i].orderQty)) + "," + anOrder[i].ticker + ",STK,SMART/" + anOrder[i].primaryExch + ",USD,DAY,MKT,Basket,U9559216,Basket,"\
				+ "FALSE,TRUE,FALSE," + today + " 06:31:00 EST,"+"5,Vwap\n";
		}

		std::cout << "Finish writing basket orders to file: " << basketFile << "\n";

		std::string anStartTime = "09:30:01 EST";
		std::string anCloseTime = "15:59:00 EST";

		testAPI.sendVWAPOrder(anOrder, 0.05, anStartTime, anCloseTime, true, false, false, 100000);

		//Sleep(1000 * 60);




#endif

#ifdef ANDV
		//std::string tradeCSVFile = "D:\\Dropbox\\Public\\Finance\\syncFile\\tradeData\\tradeData." + buffers + ".csv";
		std::string tradeCSVFile = andvTradeFilePath + "tradeData." + buffers + ".csv";
		std::cout << "Open ANDV trade file: " << tradeCSVFile << ".\n";
		
		std::vector<CSV_READ> tradeCSVRead = testAPI.getTradeCSV(tradeCSVFile);

		std::vector<STOCK_ORD> tradeOrder = testAPI.genTradeOrder(tradeCSVRead);

		if (tradeOrder.size() == 0) {
			std::cout << "There is no stock to trade today for AN/DV event. Stop program" << std::endl;
			return 1;
		}

		std::cout << "anOrder size:" << tradeOrder.size() << "\n";
		for (int i = 0; i < tradeOrder.size(); i++) {
			std::cout << "Ticker: " << tradeOrder[i].ticker << ". Primary exchange: " << tradeOrder[i].primaryExch
				<< ". share: " << tradeOrder[i].orderQty << "\n";
		}
		

		std::string tradeStartTime = "09:30:01 EST";
		std::string tradeCloseTime = "15:59:00 EST";

		testAPI.sendVWAPOrder(tradeOrder, 0.05, tradeStartTime, tradeCloseTime, true, false, false, 100000);
#endif //ANDV

#ifdef ACCSUMMARY
		int NetLiquidation = testAPI.queryNetLiquidation();
		std::cout << "NetLiquidation" << NetLiquidation << "\n";

		int buyingPower = testAPI.queryBuyingPower();
		std::cout << "BuyingPower = " << buyingPower << std::endl;

		//std::string accFile = "D:\\Cornerstone\\Account\\Account-" + buffers + ".csv";
		//std::string accFile = "D:\\Cornerstone\\Account\\accountSummary.csv";
		std::string accFile = accSummarySavePath + "accountSummary.csv";
		std::ofstream outFile;
		outFile.open(accFile, std::ios::app);

		//outFile << "NetLiquidation, BuyingPower\n";
		outFile << buffers << "," << std::to_string(NetLiquidation) << "," << std::to_string(buyingPower) << "\n";

		std::cout << "Finish writing account summary to file: " << accFile << "\n";
#endif

#ifdef HEDGEORDER
		//std::string hedgeCSVFile = "D:\\Cornerstone\\positions." + buffers + ".csv";
		//std::string hedgeCSVFile = "D:\\Cornerstone\\positions\\hedgeOrder-" + buffers + ".csv";
		std::string hedgeCSVFile = hedgeFileSavePath + "hedgeOrder-" + buffers + ".csv";
		std::vector<CSV_READ> hedgeCSVRead = testAPI.getTradeCSV(hedgeCSVFile);

		if (hedgeCSVRead.size() == 0) {
			std::cout << "There is no hedge order to trade. Stop program" << std::endl;
			return 1;
		}

		for (int i = 0; i < hedgeCSVRead.size(); i++) {
			std::cout << "Ticker: " << hedgeCSVRead[i].ticker << ". share: " << hedgeCSVRead[i].qty << "\n";
		}

		testAPI.sendFutureMktOrder(hedgeCSVRead);

		std::cout << "Finish hedgeing. " << "\n";

		
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

	//{ PrintProcessId, printf("Press return to end\n"); char s[10]; gets_s(s); }
	return 0;
}
