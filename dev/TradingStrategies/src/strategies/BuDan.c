/*
* 补单交易，在一定位置上，下，使用1MBBS ，入场。
* 不负责修改和离场
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

typedef enum trend_t
{
	UP = 1,
	DOWN = -1
}trend;

typedef enum additionalSettings_t
{
	SIDE = ADDITIONAL_PARAM_1,	
	POSITION = ADDITIONAL_PARAM_2,
	STOPLOSS = ADDITIONAL_PARAM_3
} AdditionalSettings;

typedef struct indicators_t
{
	int side;
	double position;
	double stoploss;
	int bbsTrend;	
	double bbsStopPrice;
	int bbsIndex;	
} Indicators;

AsirikuyReturnCode runBuDan(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runDuDan() failed. pParams = NULL");
		return NULL_POINTER;
	}

	loadIndicators(pParams, &indicators);

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

	setUIValues(pParams, &indicators);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Position = %ld,BBSTrend=%ld,BBStopPrice=%lf, BBSIndex = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.position, indicators.bbsTrend, indicators.bbsStopPrice, indicators.bbsIndex);

	returnCode = handleTradeEntries(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runDuDan->handleTradeEntries()", returnCode);
	}

	return SUCCESS;
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	int    shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
	int    dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;
	int    newTrend,newIndex;
	double newStopLoss;
	
	pIndicators->side = (int)parameter(SIDE);
	pIndicators->position = (double)parameter(POSITION);
	pIndicators->stoploss = (double)parameter(STOPLOSS);
		
	iBBandStop(PRIMARY_RATES, 20, 2, &pIndicators->bbsTrend, &pIndicators->bbsStopPrice, &pIndicators->bbsIndex);	

	return returnCode;
}

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators)
{	
	addValueToUI("BBSTrend", pIndicators->bbsTrend);
	addValueToUI("BBSStopPrice", pIndicators->bbsStopPrice);		
	
	return SUCCESS;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators)
{
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
	double	   stopLoss, takePrice = 0;


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

	
	if (pIndicators->bbsTrend == 1 )
	{
		stopLoss = pIndicators->stoploss;

		if (pIndicators->bbsIndex == shift1Index && pIndicators->side == 1)
		{	
			if (pParams->bidAsk.ask[0] >= pIndicators->position)
			{
				takePrice = 0;
				openSingleLongEasy(takePrice, stopLoss, 0, 1);				
			}

		}	
		
	}
	if (pIndicators->bbsTrend == -1)
	{		
		stopLoss = pIndicators->stoploss;

		if (pIndicators->bbsIndex == shift1Index && pIndicators->side == -1)
		{			
			if (pParams->bidAsk.bid[0] <= pIndicators->position)
			{

				takePrice = 0;
				openSingleShortEasy(takePrice, stopLoss, 0, 1);

			}
		}
		
	}
	return SUCCESS;
}


