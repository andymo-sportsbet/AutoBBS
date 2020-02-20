/*
* 关键K突破交易系统，可以定出日内短线的方向，一般都有消息面的支持。很可能是突破周内整理的平台。
这个时候，可以指引其他交易系统的交易方向。

if 5M, 需要在5M，5M，5M入场，这样就要检查之前两个5Mbar,是否是KK突破bar，如果是，继续入场
if 15M, 就只需要在15Mbar入场一次。
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

typedef enum trend_t
{
	UP= 1,
	RANGE=0,
	DOWN = -1
}trend;

typedef enum additionalSettings_t
{	
	KK_SL_MODE = ADDITIONAL_PARAM_1, // 0: No ModifyOrder,1:=Yes modifyorder
	KK_MULTIPLE_ATR = ADDITIONAL_PARAM_2,
	KK_ADJUSTPOINTS = ADDITIONAL_PARAM_3	
} AdditionalSettings;

typedef struct indicators_t
{	
	int dailyTrend;
	int intradayTrend;
	int bbsTrend;
	double bbsStopPrice;
	double dailyATR;
	double primaryATR;
	double primaryHigh;
	double primaryLow;
	double primaryClose;
	double primaryMovement;
	double kkStopPrice;
	double ma1H50;
	double twoDaysHigh;
	double twoDaysLow;
	double adjust;
	double risk;
} Indicators;

AsirikuyReturnCode runKeyK(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
	

	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runKeyK() failed. pParams = NULL");
		return NULL_POINTER;
	}

	loadIndicators(pParams, &indicators);	

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

	setUIValues(pParams, &indicators);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, DailyTrend = %ld,intradayTrend=%ld,kkStopPrice=%lf", 
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.dailyTrend,indicators.intradayTrend, indicators.kkStopPrice);
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preHigh = %lf,preLow=%lf, preClose = %lf,movement=%lf,dailyATR=%lf,1H50M=%lf,bbsStopPrice=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.primaryHigh, indicators.primaryLow, indicators.primaryClose, indicators.primaryMovement, indicators.dailyATR, indicators.ma1H50, indicators.bbsStopPrice);

	returnCode = handleTradeExits(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runBBS->handleTradeExits()", returnCode);
	}

	returnCode = handleTradeEntries(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runBBS->handleTradeEntries()", returnCode);
	}



	return SUCCESS;
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	int bbsIndex;	

	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
	int        dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;
		
	pIndicators->dailyATR = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pIndicators->primaryATR = iAtr(PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
		
	pIndicators->primaryHigh = iHigh(PRIMARY_RATES, 1);
	pIndicators->primaryLow = iLow(PRIMARY_RATES, 1);
	pIndicators->primaryClose = iClose(PRIMARY_RATES, 1);	
	pIndicators->intradayTrend = RANGE;
	pIndicators->kkStopPrice = -1;

	pIndicators->primaryMovement = fabs(pIndicators->primaryHigh - pIndicators->primaryLow);
	//Work out Key K 方向。如果和原来的日内方向一致，更加好。
	if (pIndicators->primaryMovement >= (double)parameter(KK_MULTIPLE_ATR) * pIndicators->dailyATR )
	{
		if (fabs(pIndicators->primaryHigh - pIndicators->primaryClose) < pIndicators->primaryMovement / 3)
		{
			pIndicators->intradayTrend = UP;
			pIndicators->kkStopPrice = pIndicators->primaryLow;
		}
		if (fabs(pIndicators->primaryLow - pIndicators->primaryClose) < pIndicators->primaryMovement / 3)
		{
			pIndicators->intradayTrend = DOWN;
			pIndicators->kkStopPrice = pIndicators->primaryHigh;
		}
	}
	pIndicators->adjust = (double)parameter(KK_ADJUSTPOINTS);
	pIndicators->ma1H50 = iMA(3, HOURLY_RATES, 50, 1);

	iBBandStop(HOURLY_RATES, 20, 2, &pIndicators->bbsTrend, &pIndicators->bbsStopPrice, &bbsIndex);	

	iSRLevels(pParams, DAILY_RATES, 2, &(pIndicators->twoDaysHigh), &(pIndicators->twoDaysLow));

	// 如果离两天高低点很近，例如<=0.5 daily ATR，就是好的入场信号，仓位不变。
	// 如果离两天的高低点远，例如> 0.5 daily ATR, 就不是很好的入场信号，仓位减半。
	pIndicators->risk = 1;

	if (pIndicators->intradayTrend == UP)
	{
		if (fabs(pIndicators->kkStopPrice - pIndicators->twoDaysLow) >= (double)parameter(KK_MULTIPLE_ATR) * pIndicators->dailyATR)
			pIndicators->risk = 0.5;		
	}
	
	if (pIndicators->intradayTrend == DOWN)
	{
		if (fabs(pIndicators->kkStopPrice - pIndicators->twoDaysHigh) >= (double)parameter(KK_MULTIPLE_ATR) * pIndicators->dailyATR)
			pIndicators->risk = 0.5;		
	}

	return returnCode;
}

static AsirikuyReturnCode iSRLevels(StrategyParams* pParams,int ratesArrayIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;

	retCode = TA_MIN(shift1Index, shift1Index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift1Index, shift1Index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}
	return SUCCESS;
}

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators)
{		
	addValueToUI("IntradayTrend", pIndicators->intradayTrend);
	addValueToUI("KKStopPrice", pIndicators->kkStopPrice);
	addValueToUI("BBStopPrice", pIndicators->bbsStopPrice);	
	addValueToUI("Movement", pIndicators->primaryMovement);
	addValueToUI("DailyATR", pIndicators->dailyATR);
	addValueToUI("PrimaryATR", pIndicators->primaryATR);
	addValueToUI("KK_SL_MODE", (int)parameter(KK_SL_MODE));
	addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
	
	return SUCCESS;
}

static AsirikuyReturnCode modifyOrders(StrategyParams* pParams, Indicators* pIndicators,OrderType orderType, double stopLoss, double takePrice)
{
	int shift0Index, shift1Index;
	time_t currentTime;
	time_t virtualOrderEntryTime;
	struct tm timeInfo1, timeInfo2;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	char       timeString2[MAX_TIME_STRING_SIZE] = "";

	int tpMode = 0;

	
	virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);

	shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
	shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
	currentTime = pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_gmtime(&timeInfo2, virtualOrderEntryTime);

	safe_timeString(timeString1, currentTime);
	safe_timeString(timeString2, virtualOrderEntryTime);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Last order update time = %s,current time = %s, Time Difference (hours) = %lf", timeString2, timeString1, difftime(virtualOrderEntryTime, currentTime) / 3600);

	if (virtualOrderEntryTime != -1 && timeInfo1.tm_mday != timeInfo2.tm_mday && timeInfo1.tm_min >= 15) // New day, because MT4 will stop for a few mins on 00：00,so need to wait for 15min. 
	{
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Move to a new day. Should modify order TP.");

		setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

		tpMode = 2;
	}
	

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			if (tpMode == 2) //如果到了新的一天，移动止损到1HBBS stop price
			{
				stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->twoDaysLow + pIndicators->adjust);
				//stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->ma1H50 + pIndicators->adjust);
				//stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->bbsStopPrice + pIndicators->adjust);
			}

			modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
		}
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			if (tpMode == 2)
			{
				stopLoss = fabs(pIndicators->twoDaysHigh - pParams->bidAsk.bid[0] + pIndicators->adjust);
				//stopLoss = fabs(pIndicators->bbsStopPrice - pParams->bidAsk.bid[0] + pIndicators->adjust);
				//stopLoss = fabs(pIndicators->ma1H50 - pParams->bidAsk.bid[0] + pIndicators->adjust);
			}

			modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
		}
	}

	return SUCCESS;	
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators)
{
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
	double	   stopLoss = -1, takePrice = 0;


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pParams = NULL");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pIndicators = NULL");
		return NULL_POINTER;
	}


	//Filter out Asia trading hours?
	//if (hour() < 9) isAisaHour = TRUE;
	
	if (pIndicators->intradayTrend == UP)
	{
		stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->kkStopPrice + pIndicators->adjust);
			
		// 1：1 
		takePrice = stopLoss;				
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk/3);

		// 1：2 
		takePrice = 2 * stopLoss;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk/3);
		
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk/3);
	}
	else if (pIndicators->intradayTrend == DOWN)
	{		
		stopLoss = fabs(pIndicators->kkStopPrice - pParams->bidAsk.bid[0] + pIndicators->adjust);
		
		// 1：1 
		takePrice = stopLoss;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk/3);

		takePrice = 2 * stopLoss;		
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk/3);

		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk/3);
	}
	//如何跟踪止损? 新的一天移动止损到1H50M？
	else
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			//if (pIndicators->bbsTrend == UP)
			//	stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->bbsStopPrice + pIndicators->adjust);
			if ((int)parameter(KK_SL_MODE))
				modifyOrders(pParams, pIndicators, BUY, stopLoss, -1);
		}
		
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			//if (pIndicators->bbsTrend == DOWN)
			//	stopLoss = fabs(pIndicators->bbsStopPrice - pParams->bidAsk.bid[0] + pIndicators->adjust);
			if ((int)parameter(KK_SL_MODE))
				modifyOrders(pParams, pIndicators, SELL, stopLoss, -1);
		}
	}
	return SUCCESS;
}

static AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pParams = NULL");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pIndicators = NULL");
		return NULL_POINTER;
	}

	if (pIndicators->intradayTrend == UP)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}
	}

	if (pIndicators->intradayTrend == DOWN)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
	}
	return SUCCESS;
}
