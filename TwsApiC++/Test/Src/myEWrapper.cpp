//============================================================================
// TwsApi Test
//============================================================================
//#include "TwsApiL0.h"			// These two headers are included in "ZGF.h"
//#include "TwsApiDefs.h"
#include "myEWrapper.h" 
using namespace TwsApi;

// to use the Sleep function
#ifdef WIN32
#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>
#define CurrentThreadId GetCurrentThreadId
#else
#include <unistd.h>			// usleep(), in microseconds
#define Sleep( m ) usleep( m*1000 )
#include <pthread.h>
#define CurrentThreadId pthread_self
#endif

#define PrintProcessId printf("%ld  ", CurrentThreadId() )

#include <time.h>



//----------------------------------------------------------------------------
// MyEWrapper
//----------------------------------------------------------------------------

MyEWrapper::MyEWrapper() : EWrapperL0(true) {
	b_nextValidId = false; 
	b_posReady = false;
	b_openOrdReady = false;
	b_accSummary = false;
	tickData.resize(10); 
	//stockPos.resize(10);
}
//MyEWrapper( bool CalledFromThread = true ): EWrapperL0( CalledFromThread ). Use default tread true constructor.

void MyEWrapper::waitForNextValidId() {
	while (!b_nextValidId) {
		std::cout << "nextValidId is not ready." << std::endl;
		Sleep(1000);
	}
}

void MyEWrapper::init_tickData() {
	size_t size = tickData.size();
	tickData.clear();
	tickData.resize(size);
}


void MyEWrapper::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute)
{
		///	int id = 10, y = 0; id = id/y; // Divide by zero to test EWrapperL0Protected
		///	int *id = 0; *id = 100;

		///	throw(0);

		//	time_t		_t	= GetEventTime();		// !! from L1
		time_t		_t; time(&_t);
		struct tm*	_tm = localtime(&_t);
		
		/*
		PrintProcessId, printf
		("TP: %4ld %02d:%02d:%02d %10s %5.3f\n"
			, tickerId
			, _tm->tm_hour, _tm->tm_min, _tm->tm_sec
			, *(TickTypes::ENUMS)field, price
		);
		*/

		switch (field) {
		case BID: 
			tickData[tickerId].bidPrice[0] = price; 
			//std::cout << "bid = " << tickData[tickerId].bidPrice[0] << std::endl;
			break;
		case ASK: 
			tickData[tickerId].askPrice[0] = price; 
			//std::cout << "ask = " << tickData[tickerId].askPrice[0] << std::endl;
			break;
		}
		
		
}

void MyEWrapper::tickSize(TickerId tickerId, TickType field, int size)
{
		//	time_t		_t	= GetEventTime();		// !! from L1
	time_t		_t; time(&_t);
	struct tm*	_tm = localtime(&_t);
	/*
	PrintProcessId, printf
	("TS: %4ld %02d:%02d:%02d %10s %5d\n", tickerId, _tm->tm_hour, _tm->tm_min, _tm->tm_sec, *(TickTypes::ENUMS)field, size
		);
	*/	

	switch (field) {
	case BID_SIZE:
		tickData[tickerId].bidSize[0] = size; 
		//std::cout << "bidSize = " << tickData[tickerId].bidSize[0] << std::endl;
		break;
	case ASK_SIZE:
		tickData[tickerId].askSize[0] = size; 
		//std::cout << "askSize = " << tickData[tickerId].askSize[0] << std::endl;
		break;
	}
}

void MyEWrapper::winError(const IBString& str, int lastError)
{
	PrintProcessId, printf("WinError: %d = %s\n", lastError, (const char*)str);
}

void MyEWrapper::connectionClosed()
{
	PrintProcessId, printf("Connection Closed\n");
}

void MyEWrapper::updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName)
{
	UpdateAccountValueKey::ENUMS UAVK;
	if (UAVK *= key)
		PrintProcessId, printf("AC: %s %5s   %-30s %s\n", (const char*)accountName, (const char*)currency, *UAVK, (const char*)val);
	else
		PrintProcessId, printf("AC: %s %5s ? %-30s %s\n", (const char*)accountName, (const char*)currency, (const char*)key, (const char*)val);
}

void MyEWrapper::updatePortfolio(const Contract& contract, int position, double marketPrice, double marketValue, double averageCost,\
		double unrealizedPNL, double realizedPNL, const std::string& accountName) 
{
	printf("UpdatePortfolio. %s, %s @ %s: Position: %d, MarketPrice: %g, MarketValue: %g, AverageCost: %g, UnrealisedPNL: %g, RealisedPNL: %g, AccountName: %s\n", \
		(contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), position, marketPrice, \
		marketValue, averageCost, unrealizedPNL, realizedPNL, accountName.c_str());
}

void MyEWrapper::position(const IBString& account, const Contract& contract, int position, double avgCost)
{
	/*
	printf("Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %d, Avg Cost: %g\n", \
		account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), position, avgCost);
*/
	STOCK_POS tmpPos = { contract.symbol.c_str(), avgCost, position };

	stockPos.push_back(tmpPos);
}

void MyEWrapper::positionEnd()
{
	//printf("PositionEnd\n");

	b_posReady = true;
}

void MyEWrapper::accountDownloadEnd(const IBString& accountName)
{
	PrintProcessId, printf("AC: %s end\n", (const char*)accountName);
}

void MyEWrapper::nextValidId(OrderId orderId)
{
	b_nextValidId = true;
	m_orderId = orderId;
	PrintProcessId, printf("nextValidId = %ld\n", m_orderId);
}

void MyEWrapper::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState)
{
	/*
	PrintProcessId, printf("OpenOrder. ID: %ld, %s, %s @ %s: %s, %s, %ld, %s\n", orderId, contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(), \
		order.action.c_str(), order.orderType.c_str(), order.totalQuantity, orderState.status.c_str());
		*/

	OPEN_ORD tmpOrd = { orderId, contract.symbol.c_str(), order.action.c_str(), order.totalQuantity };
	openOrd.push_back(tmpOrd);
}

void MyEWrapper::orderStatus(OrderId orderId, const IBString& status, int filled, int remaining, double avgFillPrice, int permId, int parentId, \
	double lastFillPrice, int clientId, const IBString& whyHeld)
{
	/*
	PrintProcessId, printf("OrderStatus. ID: %ld, Status: %s, Filled: %d, Remaining: %d, AvgFillPrice: %g, PermId: %d, LastFillPrice: %g, ClientId: %d, WhyHeld: %s\n", \
		orderId, status.c_str(), filled, remaining, avgFillPrice, permId, lastFillPrice, clientId, whyHeld.c_str());
		*/
}

void MyEWrapper::openOrderEnd()
{
	//printf("OpenOrderEnd\n");
	b_openOrdReady = true;
}


void MyEWrapper::contractDetails(const ContractDetails& contractDetails)
{
	const Contract& C = contractDetails.summary;

	PrintProcessId, printf("CD: %10s %5s %8s, %5.2f\n", (const char*)C.localSymbol, (const char*)C.secType, (const char*)C.expiry, C.strike);
}

void MyEWrapper::error(const int id, const int errorCode, const IBString errorString)
{
	PrintProcessId, printf("Error for id=%d: %d = %s\n", id, errorCode, (const char*)errorString);
}

void MyEWrapper::historicalData(TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps)
{
	if (IsEndOfHistoricalData(date))
	{
		PrintProcessId, printf("HD: %s\n", Finished());
		return;
	}

	PrintProcessId, printf("HD: %4ld %10s %5.3f %5.3f %5.3f %5.3f %7d %7d\n", reqId, (const char*)date, open, high, low, close, volume, barCount);
}

void MyEWrapper::updateMktDepth(TickerId id, int position, int operation, int side, double price, int size)
{
	PrintProcessId, printf
	("MD: %4ld %2d %10s %5s %7.2f %5d\n"
		, id
		, position
		, *(MktDepthOperation::ENUMS) operation
		, *(MktDeptSide::ENUMS)side
		, price
		, size
	);
}

void MyEWrapper::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation, int side, double price, int size)
{
}

void MyEWrapper::connectionOpened(void)
{
	PrintProcessId, printf("Connection Opened\n");
}

void MyEWrapper::checkMessagesStarted(void)
{
	PrintProcessId, printf(">>> checkMessagesStarted\n");
}

void MyEWrapper::checkMessagesStopped(void)
{
	PrintProcessId, printf("<<< checkMessagesStopped\n");
}

void MyEWrapper::accountSummary(int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& currency) 
{
	//printf("Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
	b_accSummary = false;
	std::string tmpTag = tag.c_str();

	if (tmpTag == "CashBalance") {
		cashBalance = std::stoi(value.c_str());
		//std::cout << "CashBalance = "  << cashBalance <<std::endl;
	}
}

void MyEWrapper::accountSummaryEnd(int reqId) 
{
	//printf("AccountSummaryEnd. Req Id: %d\n", reqId);
	b_accSummary = true;
}



//----------------------------------------------------------------------------
// TestEnums
//----------------------------------------------------------------------------
void TestEnums(void)
{
	IBString			id = *TickTypes::Bid;
	id = *OrderStatus::PendingCancel;

	IBString			y;
	y = *TriggerMethod::LastPrice;

	OrderStatus::ENUMS	e;
	if (e *= "PendingCancel")
	{
		printf("OK\t");
	}
	else
	{
		printf("NOK\t");
	}
	printf("%4d %s\n", e, *e);


	if (e *= "not a status")
	{
		printf("OK\t");
	}
	else
	{
		printf("NOK\t");
	}
	printf("%4d %s\n", e, *e);

	switch (OrderStatus("PendingCancelxxxx"))
	{
	case OrderStatus::PendingCancel:
	{ printf("OK\n"); } break;
	case OrderStatus::_INVALID_:
	{ printf("NOK\n"); } break;
	default:
	{ printf("??\n"); } break;
	}

	// The iterator has a similar interface as the of the std::map
	for (UpdateAccountValueKey::iterator ac = UpdateAccountValueKey::begin(); ac != UpdateAccountValueKey::end(); ++ac)
		printf("%4d %s\n", ac->first, ac->second);

}



//----------------------------------------------------------------------------
// Marco TEST
// Prints out the statement before executing it.
// Used just to demonstrate the api: you see the statement, and then the result
// i.e.: TEST(id, EC->reqMktData( id, C, "", false ) );
// Without TEST: EC->reqMktData( id, C, "", false );
//----------------------------------------------------------------------------
#define TEST( T, X ) ( printf( "T%7d %s\n", T, #X ), X )

//----------------------------------------------------------------------------
// main
//----------------------------------------------------------------------------


struct Contract_ : public Contract
{
	Contract_(IBString sb, IBString st, IBString cr, IBString ex, IBString pr_ex)
		: Contract()
	{
		symbol = sb;
		secType = st;		//"STK"
		currency = cr;
		exchange = ex;	  	//"SMART";
		primaryExchange = pr_ex;	//"ISLAND";
	}
};

//Contract_			C("AAPL", *SecType::STK, "USD", *Exchange::IB_SMART, *Exchange::ISLAND);

int mainM(void)
{
	printf("APIVersion    = %s\n", EClientL0::apiVersion());

	//	TestEnums();

	//Contract_			C( "MSFT", *SecType::STK, "USD", *Exchange::IB_SMART );

	/*
	Contract			C;
	C.symbol			= "MSFT";
	C.secType			= *SecType::STK;		//"STK"
	C.currency			= "USD";
	C.exchange			= *Exchange::IB_SMART;	//"SMART";
	//	C.primaryExchange	= *Exchange::AMEX;
	*/
	// from version 9.63 on, the protected ewrapper is active by default
	MyEWrapper	MW;
	EClientL0*	EC = EClientL0::New(&MW);

	std::vector<std::string> tickerList = { "AAPL","FB","AMZN" };

	printf("ClientVersion = %d\n", EC->clientVersion());

	std::cout << "nextValidId? = " << MW.b_nextValidId << std::endl;

	TEST(0, EC->eDisconnect());	// only for test purposes

	if (TEST(0, EC->eConnect("", 7497, 100)))
	{
		PrintProcessId, printf("ServerVersion = %d\n", EC->serverVersion());

		// pause the program until nextValidId is ready
		MW.waitForNextValidId();

		//		EC->reqNewsBulletins( true );
		//		EC->reqNewsBulletins( true );
		
		//TEST( 0, EC->reqAccountUpdates( true, "" ) );
		

		//		for( int i = 0; i < 60; i++ )
		//TEST( 100, EC->reqMktData( 100, C, "", false ) );
		/*
		EC->reqMktDepth( 11, C, 3 );
		*/
		/*
		EC->reqHistoricalData
		(20
			, C
			, EndDateTime(2014, 8, 4)
			, DurationStr(1, *DurationHorizon::Days)
			, *BarSizeSetting::_1_hour
			, *WhatToShow::TRADES
			, true
			, FormatDate::AsDate
		);
		*/
		
		TEST(0, EC->reqMktData(0, ContractSamples::USStock("AAPL"),"", false));

		//TEST(55, EC->placeOrder(MW.m_orderId++, C, OrderSamples::LimitOrder("SELL", 300, 40)));
		//TEST(55, EC->placeOrder(MW.m_orderId++, C, OrderSamples::LimitOrder("BUY", 120, 935)));

		//TEST(40, EC->reqOpenOrders());

		TEST(41, EC->reqPositions());


		//TEST(55, EC->placeOrder(MW.m_orderId++, ContractSamples::USStock("NVDA"), OrderSamples::LimitOrder("BUY", 300, 400)));
		//TEST(42, EC->reqPositions());

		//TEST(11, EC->reqGlobalCancel());

		//TEST(41, EC->reqGlobalCancel());

		//TEST(42, EC->reqOpenOrders());

		/*
	
		EC->reqHistoricalData
		( 20//EC->GetNextValidId()
		, C
		, EndDateTime(2006,10,03), DurationStr(1, DH_Days), *BarSizeSetting::_1_secs
		, *WS_TRADES
		, true
		, FD_AsDate
		);
		*/

		/*

		{
			Contract C;
			C.symbol = "FB";
			C.secType = *SecType::OPT;		//"STK"
			C.currency = "USD";
			C.exchange = *Exchange::IB_SMART;	//"SMART";
			EC->reqContractDetails(25, C);


			//		EC->reqContractDetails( C );
		}
		*/

		/*  QQQQ
		C.symbol	= "DNEX";
		EC->reqContractDetails( C );

		C.symbol	= "MSFT";
		EC->reqContractDetails( C );
		*/
	}

	int id = 1;
	while (id++ < 1000)
	{
		//	if( !MW.IsCalledFromThread() ) EC->checkMessages();
		Sleep(100);

		/* for 'stress' testing */
		//if (0 == id % 50)
			//TEST(id, EC->reqMktData( id, C, "", false ) );
			/**/
			//if (30 == id)
				//TEST(id, EC->eDisconnect());
		/**/
		//if (40 == id)
			//TEST(id, EC->eDisconnect());


		//if (60 == id || 70 == id)
			//TEST(id, EC->reqMktData(id, ContractSamples::USStock("AAPL"), "", false));	// will fail of course


		//if (130 == id)
			//TEST(id, EC->eConnect("", 7497, 10));

		//if (150 == id)
			//TEST(id, EC->eConnect("", 7497, 10));		// raises already connected error

		if (190 == id)
			TEST(id,
				EC->reqHistoricalData
				(id
					, ContractSamples::USStock("NVDA")
					, EndDateTime(2015, 02, 20), DurationStr(1, *DurationHorizon::Days), *BarSizeSetting::_1_hour
					, *WhatToShow::TRADES
					, true
					, FormatDate::AsDate
				)
			);

		//if (200 == id)
			//TEST(id, EC->reqMktData(id, ContractSamples::USStock("BABA"), "", false));

		/**/
		//if (800 == id)
			//TEST(id, EC->cancelMktData(200));
		/**/
	}


	TEST(0, EC->eDisconnect());

	delete EC;

	{ PrintProcessId, printf("Press return to end\n"); char s[10]; gets_s(s); }
	return 0;
}
