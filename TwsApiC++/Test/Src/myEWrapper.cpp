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
	b_ctrDetail = false;
	m_orderId = -1;
	minTick = 0;
	
	n_quoteStatus.resize(30);
	tickData.resize(30); 
}
//MyEWrapper( bool CalledFromThread = true ): EWrapperL0( CalledFromThread ). Use default thread true constructor.

void MyEWrapper::init_tickData() {
	size_t size = tickData.size();
	tickData.clear();
	tickData.resize(size);

	size_t size1 = n_quoteStatus.size();
	n_quoteStatus.clear();
	n_quoteStatus.resize(size1);
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
			n_quoteStatus[tickerId].n_bidprice++;
			//std::cout << "bid = " << tickData[tickerId].bidPrice[0] << std::endl;
			break;
		case ASK: 
			tickData[tickerId].askPrice[0] = price; 
			n_quoteStatus[tickerId].n_askprice++;
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
		n_quoteStatus[tickerId].n_bidsize++;
		//std::cout << "bidSize = " << tickData[tickerId].bidSize[0] << std::endl;
		break;
	case ASK_SIZE:
		tickData[tickerId].askSize[0] = size; 
		n_quoteStatus[tickerId].n_asksize++;
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
	POS tmpPos = { contract.symbol.c_str(), avgCost, position, contract.secType.c_str() };

	allPos.push_back(tmpPos);
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
	//PrintProcessId, printf("nextValidId = %ld\n", m_orderId);
}

void MyEWrapper::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState)
{
	/*
	PrintProcessId, printf("OpenOrder. ID: %ld, %s, %s @ %s: %s, %s, %ld, %s\n", orderId, contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(), \
		order.action.c_str(), order.orderType.c_str(), order.totalQuantity, orderState.status.c_str());
		*/
	OPEN_ORD tmpOrd = { contract.symbol.c_str(), order.action.c_str(), order.totalQuantity };
	combOpenOrd[orderId].openOrd = tmpOrd;
}

void MyEWrapper::orderStatus(OrderId orderId, const IBString& status, int filled, int remaining, double avgFillPrice, int permId, int parentId, \
	double lastFillPrice, int clientId, const IBString& whyHeld)
{
	/*
	PrintProcessId, printf("OrderStatus. ID: %ld, Status: %s, Filled: %d, Remaining: %d, AvgFillPrice: %g, PermId: %d, LastFillPrice: %g, ClientId: %d, WhyHeld: %s\n", \
		orderId, status.c_str(), filled, remaining, avgFillPrice, permId, lastFillPrice, clientId, whyHeld.c_str());
		*/
	ORD_STATUS tmpStatus = { status.c_str() , filled ,remaining, avgFillPrice, lastFillPrice, clientId};
	combOpenOrd[orderId].ordStatus = tmpStatus;
}

void MyEWrapper::openOrderEnd()
{
	//printf("OpenOrderEnd\n");
	b_openOrdReady = true;
}


void MyEWrapper::contractDetails(int reqId,const ContractDetails& contractDetails)
{
	//const Contract& C = contractDetails.summary;

	//std::cout << "min Tick = " << contractDetails.minTick << std::endl;
	
	minTick = contractDetails.minTick;
	primaryExchange = contractDetails.summary.primaryExchange.c_str();
	//PrintProcessId, printf("CD: %10s %5s %8s, %5.2f\n", (const char*)C.localSymbol, (const char*)C.secType, (const char*)C.expiry, C.strike);
}

void MyEWrapper::contractDetailsEnd(int reqId)
{
	//printf("ContractDetailsEnd. %d\n", reqId);
	b_ctrDetail = true;
	//std::cout << "n_ctrDetail = " << n_ctrDetail << std::endl;

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

void MyEWrapper::fundamentalData(TickerId reqId, const IBString& data) {
	std::cout << "fundamental " << std::endl;
	printf("FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
}