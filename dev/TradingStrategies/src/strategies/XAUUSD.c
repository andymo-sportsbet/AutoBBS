#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "StrategyUserInterface.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSEF

typedef enum additionalSettings_t
{
	XAUUSD_ADJUSTPOINTS = ADDITIONAL_PARAM_1,	
	XAUUSD_TREND_MODE = ADDITIONAL_PARAM_2, 
	XAUUSD_DAILY_ATR = ADDITIONAL_PARAM_3,
	XAUUSD_WEEKLY_ATR = ADDITIONAL_PARAM_4,
	XAUUSD_MAX_ACCOUNT_RISK = ADDITIONAL_PARAM_4, 
	XAUUSD_MAX_STRATEGY_RISK = ADDITIONAL_PARAM_5,
	XAUUSD_MACRO_TREND = ADDITIONAL_PARAM_6, //来自整年的长期趋势的基本面分析
	XAUUSD_EXECUTION_RATES = ADDITIONAL_PARAM_7, // 明确指出BBS的执行时间框架，例如15，30
	XAUUSD_ONE_SIDE = ADDITIONAL_PARAM_8,  // 1: Buy only, -1: Sell only, 0: either	
	XAUUSD_IS_AUTO_MODE = ADDITIONAL_PARAM_9  // 1: apply for the filter for daily trading, 0: ignore the filter, that is manual mode
} AdditionalSettings;


typedef struct indicators_t
{
	//BBS indicators	
	int side;
	int macroTrend;
	int executionTrend;
	int entrySignal;
	int exitSignal;
	int orderManagement;
	int tpMode;
	int splitTradeMode;
	int tradeMode;

	int bbsTrend_excution;
	double bbsStopPrice_excution;
	int bbsIndex_excution;

	int bbsTrend_primary;	
	double bbsStopPrice_primary;	
	int bbsIndex_primary;

	int bbsTrend_secondary;
	double bbsStopPrice_secondary;
	int bbsIndex_secondary;

	int bbsTrend_1H;
	double bbsStopPrice_1H;
	int bbsIndex_1H;

	int bbsTrend_4H;
	double bbsStopPrice_4H;
	int bbsIndex_4H;

	int bbsTrend_Daily;
	double bbsStopPrice_Daily;
	int bbsIndex_Daily;

	double entryPrice;
	double stopLossPrice;
	double risk;

	double primaryATR;
	double adjust;

	double strategyMarketVolRisk;
	double strategyRisk;
	double strategyRiskWithoutLockedProfit;
	double riskPNL;
	double riskPNLWithoutLockedProfit;
	double limitRiskPNL;
	double strategyMaxRisk;
	double strategyMaxDD;

	int executionRateTF;

	double total_lose_pips;
	int lossTimes;
	int winTimes;

	double asia_high;
	double asia_low;
	double asia_close;
	double asia_open;

	double euro_high;
	double euro_low;
	double euro_close;
	double euro_open;

} Indicators;

static int filterExcutionTF(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	struct tm timeInfo1;
	time_t currentTime;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	//Only work for 15M and always use secondary rates, in this case, it should be 15M
	if ((int)pParams->settings[TIMEFRAME] != pIndicators->executionRateTF) //it means 15-25M		
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_secondary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_secondary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_secondary;
		shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

		if (timeInfo1.tm_min % pIndicators->executionRateTF >= 3)
			return SUCCESS;		
	}
	else
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_primary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_primary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_primary;
		shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	}
	return shift1Index;
}

AsirikuyReturnCode runXAUUSD(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;
	Base_Indicators base_Indicators;
	int rateErrorTimes = -1;
	BOOL isRateCheck = TRUE;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runAutoBBS() failed. pParams = NULL");
		return NULL_POINTER;
	}

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	if (strcmp(timeString, "19/07/17 11:00") == 0)
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, "hit a point");

	if (strcmp(timeString, "29/09/17 16:30") == 0)
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, "hit a point");

	// Read rateError.txt file to get validateSecondaryBarsGap error times. 
	// If more than 3 times, skip checking. 

	rateErrorTimes = readRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);


	if (rateErrorTimes >= 2)
		isRateCheck = FALSE;


	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE &&
			(
			validateCurrentTimeEasy(pParams, B_PRIMARY_RATES) > 0 ||
			validateDailyBarsEasy(pParams, B_PRIMARY_RATES, B_DAILY_RATES) > 0 ||
			validateHourlyBarsEasy(pParams, B_PRIMARY_RATES, B_HOURLY_RATES) > 0 ||
			validateSecondaryBarsEasy(pParams, B_PRIMARY_RATES, B_SECONDARY_RATES, (int)parameter(XAUUSD_EXECUTION_RATES), isRateCheck) > 0
			)
		)	
		return SUCCESS;
		

	// if passed, reset number to 0. 
	if (isRateCheck == TRUE)
		resetRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

	if ((int)parameter(XAUUSD_MACRO_TREND) * (int)parameter(AUTOBBS_ONE_SIDE) < 0)
	{
		pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Invalid paramenter config: System InstanceID = %d, BarTime = %s, AUTOBBS_MACRO_TREND= %d：AUTOBBS_ONE_SIDE=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, (int)parameter(AUTOBBS_MACRO_TREND), (int)parameter(AUTOBBS_ONE_SIDE));
		return INVALID_CONFIG;
	}

	// Load the base indicators
	runBase(pParams, &base_Indicators);

	loadIndicators(pParams, &indicators, &base_Indicators);

	setUIValues(pParams, &indicators, &base_Indicators);


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_primary=%ld,BBStopPrice_primary=%lf, BBSIndex_primary = %ld", 
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_primary, indicators.bbsStopPrice_primary, indicators.bbsIndex_primary);
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,bbsTrend_secondary=%ld,BBStopPrice_secondary=%lf, bbsIndex_secondary = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_secondary, indicators.bbsStopPrice_secondary, indicators.bbsIndex_secondary);
	
	returnCode = handleTradeExits(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeExits()", returnCode);
	}

	returnCode = handleTradeEntries(pParams, &indicators,&base_Indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeEntries()", returnCode);
	}	

	return SUCCESS;
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	pIndicators->primaryATR = iAtr(B_PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	iBBandStop(B_PRIMARY_RATES, 20, 2, &pIndicators->bbsTrend_primary, &pIndicators->bbsStopPrice_primary, &pIndicators->bbsIndex_primary);
	iBBandStop(B_SECONDARY_RATES, 20, 2, &pIndicators->bbsTrend_secondary, &pIndicators->bbsStopPrice_secondary, &pIndicators->bbsIndex_secondary);	
	
	pIndicators->adjust = (double)parameter(AUTOBBS_ADJUSTPOINTS);
	pIndicators->risk = 1;
	pIndicators->entrySignal = 0;
	pIndicators->exitSignal = EXIT_NONE;
	pIndicators->executionTrend = 0;
	pIndicators->orderManagement = 0;
	pIndicators->tpMode = (int)parameter(AUTOBBS_LONG_SHORT_MODE);
	pIndicators->splitTradeMode = (int)parameter(AUTOBBS_TREND_MODE);
	pIndicators->stopLossPrice = 0;
	pIndicators->macroTrend = (int)parameter(AUTOBBS_MACRO_TREND);
	pIndicators->side = (int)parameter(AUTOBBS_ONE_SIDE);
	pIndicators->executionRateTF = (int)parameter(AUTOBBS_EXECUTION_RATES);

	pIndicators->tradeMode = 1;

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[ACCOUNT_RISK_PERCENT] * -1;
	
	pIndicators->total_lose_pips = 0;

	workoutExecutionTrend(pParams, pIndicators, pBase_Indicators);

	return returnCode;
}


static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{

	switch ((int)parameter(AUTOBBS_TREND_MODE)) {		
	case 15:
	case 16:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_secondary);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_secondary);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		addValueToUI("riskPNL", pIndicators->riskPNL);
		addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	default:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("DailyS", pBase_Indicators->dailyS);	
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		addValueToUI("riskPNL", pIndicators->riskPNL);
		addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	}

	return SUCCESS;
}

static void profitManagement_base(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{

	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);
	
	// Control risk by Macro factor
	if (pIndicators->entrySignal == 1)
	{
		if (pIndicators->macroTrend < 0)
		{
			pIndicators->risk *= 0.5;
			pIndicators->strategyMaxRisk *= 0.5;
		}
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= 0.8;
			pIndicators->strategyMaxRisk *= 0.8;
		}
	}

	if (pIndicators->entrySignal == -1)
	{
		if (pIndicators->macroTrend > 0)
		{
			pIndicators->risk *= 0.5;
			pIndicators->strategyMaxRisk *= 0.5;
		}
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= 0.8;
			pIndicators->strategyMaxRisk *= 0.8;
		}
	}

	if (pIndicators->entrySignal != 0 && pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->strategyMaxRisk)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, strategyRisk %lf： strategyRiskWithoutLockedProfit %lf, skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRisk, pIndicators->strategyRiskWithoutLockedProfit,pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}
		
	if (pIndicators->riskPNL <  pIndicators->limitRiskPNL && pIndicators->entrySignal != 0)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, PNL riks %lf：riskPNLWithoutLockedProfit %lf, skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit,pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}


}


static void profitManagement(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	int   noTPOrderDaysNumber = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];

	safe_timeString(timeString, currentTime);

	profitManagement_base(pParams, pIndicators, pBase_Indicators);
	noTPOrderDaysNumber = getSamePricePendingNoTPOrdersEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3);

	if (noTPOrderDaysNumber >= 4 && pIndicators->entrySignal != 0)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, SamePricePendingNoTPOrdersDays %d, skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, noTPOrderDaysNumber, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	// Daily Profit on EOD?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->dailyR3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->dailyS3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);


	// Weekly Profit on EOW?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);


	// when floating profit is too high, fe 10%
	if (pBase_Indicators->dailyTrend_Phase > 0
		&& pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2
		&& pIndicators->riskPNL >targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	else if (pBase_Indicators->dailyTrend_Phase < 0
		&& pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2
		&& pIndicators->riskPNL > targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);

	//closeAllWithNegativeEasy(3, currentTime, 3);
	//closeAllWithNegativeEasy(2, currentTime,3);

	//// work out the Max DD
	//pIndicators->strategyMaxDD = pIndicators->riskPNL - pIndicators->strategyRisk;
	//if (pIndicators->strategyMaxDD > parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3)
	//{
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Max DD %lf",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyMaxDD);

	//	closeWinningPositionsEasy(pIndicators->riskPNL, pIndicators->riskPNL);
	//}

}

static BOOL isNewDay(StrategyParams* pParams, time_t currentTime)
{	
	time_t virtualOrderEntryTime;
	struct tm timeInfo1, timeInfo2;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	char       timeString2[MAX_TIME_STRING_SIZE] = "";

	virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);
	
	safe_gmtime(&timeInfo1, currentTime);
	safe_gmtime(&timeInfo2, virtualOrderEntryTime);

	safe_timeString(timeString1, currentTime);
	safe_timeString(timeString2, virtualOrderEntryTime);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Last order update time = %s,current time = %s, Time Difference (hours) = %lf", timeString2, timeString1, difftime(virtualOrderEntryTime, currentTime) / 3600);

	if (virtualOrderEntryTime != -1 && timeInfo1.tm_mday != timeInfo2.tm_mday && timeInfo1.tm_min >= 15) // New day
	{
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Move to a new day.");
		//setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);
		return TRUE;
	}
	return FALSE;
}

/*
TODO: 如何动态修改不同order的止损和止盈？
最好就在策略里面直接call modifyTradeEasy_DayTrading()
*/
static AsirikuyReturnCode modifyOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators,OrderType orderType, double stopLoss, double takePrice)
{
	int tpMode = 0;
	int shift0Index;
	time_t currentTime;	
	double stopLoss2;	

	shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];


	if (orderType == BUY && totalOpenOrders(pParams, BUY) > 0)
		modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode); 

	if (orderType == SELL && totalOpenOrders(pParams, SELL) > 0)	
		modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode); 	
}

static void splitBuyOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots,lots_singal;
	double gap = pHigh - pIndicators->entryPrice;

	takePrice = 3;

	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice);

	//lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;	
	//if (pIndicators->total_lose_pips > takePrice * lots)
	{
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;

		////预计2倍的风险,单次最高3% risk
		if (lots *  takePrice * 100  / pParams->accountInfo.equity > 0.015)
			lots = pParams->accountInfo.equity * 0.015 / 100 / takePrice;
	}

	openSingleLongEasy(takePrice, stopLoss, lots, 0);
	
}

static void splitSellOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal;
	double gap = pIndicators->entryPrice - pLow;


	takePrice = 3;
	lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice);
	//lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;		

	//if (pIndicators->total_lose_pips > takePrice * lots)
	{
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;

		////预计2倍的风险,单次最高3% risk
		if (lots *  takePrice * 100 / pParams->accountInfo.equity > 0.015)
			lots = pParams->accountInfo.equity * 0.015 / 100 / takePrice;
	}

	openSingleShortEasy(takePrice, stopLoss, lots, 0);
	
}


// Short term trades only
static void splitBuyOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pHigh - pIndicators->entryPrice;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	double riskcap = parameter(AUTOBBS_RISK_CAP);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	//至少有1/3 空间
	
	if (pIndicators->tradeMode == 1 && gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) <  pBase_Indicators->dailyATR)
	{

		takePrice = gap / 3;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		// Cap to 2 % risk
		//if (stopLoss / takePrice * pParams->settings[ACCOUNT_RISK_PERCENT] > riskcap)
		//	lots = lots * riskcap / (stopLoss / takePrice * pParams->settings[ACCOUNT_RISK_PERCENT]);
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}

	{
		if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
		{
			takePrice = 0;
			if(pIndicators->tradeMode == 1)
				openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
			else
				openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
		}
	}


	
}

static void splitBuyOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	//至少有1/3 空间
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);		
	}
}


static void splitSellOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	//至少有1/3 空间
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}


// Short term trades only
// split into 3 trades
// 33% 1:1
// 33% 2:1
// 33% no tp or 3:1
static void splitSellOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pIndicators->entryPrice - pLow;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	time_t currentTime;
	struct tm timeInfo1;

	double riskcap = parameter(AUTOBBS_RISK_CAP);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	//至少有1/3 空间
	if (pIndicators->tradeMode == 1 && gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
	{
		takePrice = gap / 3;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;			
		openSingleShortEasy(takePrice, stopLoss, lots, 0);

	}

	{
		if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
		{
			takePrice = 0;
			if(pIndicators->tradeMode == 1)
				openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
			else
				openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
		}
	}
	
}


static void splitBuyOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode) {
		case 0:
			splitBuyOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;							
		case 1:
			splitBuyOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;		
		case 2:
			splitBuyOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
	}
	
}

static void splitSellOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode) {
	case 0:
		splitSellOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 1:
		splitSellOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 2:
		splitSellOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;		
	}
}


static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	double	   stopLoss, takePrice_primary = 0;
	int riskcap = (int) parameter(AUTOBBS_RISK_CAP);


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
	
	// When the trade is created from 23:45 to 00:20, increase the 3 times of spread for cut over
	if ((hour() == 23 && minute() > 40) || (hour() == 00 && minute() < 20)) 
		pIndicators->adjust = 3 * pIndicators->adjust;

	stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) + pIndicators->adjust;

	switch (pIndicators->tpMode) {
		case 0:
			takePrice_primary = stopLoss;
			break;
		case 1:		
			takePrice_primary = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_secondary) + pIndicators->adjust;
			pIndicators->risk = pIndicators->risk * min(stopLoss / takePrice_primary, riskcap);
			break;
		case 2:
			takePrice_primary = 0;
			break;
		case 3:
			takePrice_primary = pBase_Indicators->dailyATR;
			break;
		default:
			takePrice_primary = stopLoss;
			break;
	}
	
	if (pIndicators->executionTrend == 1)
	{	
		if (pIndicators->entrySignal == 1)
				splitBuyOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);				
		else if (totalOpenOrders(pParams, BUY) > 0)
			modifyOrders(pParams, pIndicators, pBase_Indicators, BUY, stopLoss, -1);
	}

	if (pIndicators->executionTrend == -1)
	{	
		if (pIndicators->entrySignal == -1)		
			splitSellOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);		
		else if (totalOpenOrders(pParams, SELL) > 0)
			modifyOrders(pParams, pIndicators, pBase_Indicators, SELL, stopLoss, -1);
		
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

	switch (pIndicators->exitSignal)
	{
		case EXIT_SELL:
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				closeAllShorts();
			}
			break;
		case EXIT_BUY:
			if (totalOpenOrders(pParams, BUY) > 0)
			{
				closeAllLongs();
			}
			break;
		case EXIT_ALL:
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				closeAllShorts();
			}

			if (totalOpenOrders(pParams, BUY) > 0)
			{
				closeAllLongs();
			}
			break;
	}
	
	return SUCCESS;
}

/*
分析系统：
根据设置，分别有：
1. 趋势自动跟踪系统，内涵长短单，日内重仓交易。
2. 自动对冲系统，很少发生，但是成功率高。 需要进一步研究对冲系统。
3. 纯日内交易系统，带有反手翻倍加仓。这里面最大风险如何控制，还没有很好的办法。只是依靠时间和日内过滤来规避风险。
*/
static AsirikuyReturnCode workoutExecutionTrend(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	struct tm timeInfo;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int   startHour = 0;

	if (strstr(pParams->tradeSymbol, "XAU") != NULL)
		startHour = 1;

	safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	//Strategy risk
	pIndicators->strategyMarketVolRisk = caculateStrategyVolRiskEasy(pBase_Indicators->dailyATR);
	
	pIndicators->strategyRisk = caculateStrategyRiskEasy(FALSE);
	pIndicators->strategyRiskWithoutLockedProfit = caculateStrategyRiskEasy(TRUE);

	pIndicators->riskPNL = caculateStrategyPNLEasy(FALSE);
	pIndicators->riskPNLWithoutLockedProfit = caculateStrategyPNLEasy(TRUE);

	switch ((int)parameter(AUTOBBS_TREND_MODE)) {					
		case 1:
			workoutExecutionTrend_Auto(pParams, pIndicators, pBase_Indicators);			
			break;
		case 2:
			workoutExecutionTrend_Auto_Hedge(pParams, pIndicators, pBase_Indicators);			
			break;
		case 3:
			workoutExecutionTrend_DayTrading(pParams, pIndicators, pBase_Indicators);
			break;		
	}


	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && pParams->accountInfo.totalOpenTradeRiskPercent < parameter(AUTOBBS_MAX_ACCOUNT_RISK) * -1) //if account risk is more than 3%, stop entring trades.
	{		
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Over max riks %lf： skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, parameter(AUTOBBS_MAX_ACCOUNT_RISK),pIndicators->entrySignal );
		pIndicators->entrySignal = 0;
	}

	//Filter out side
	if (pIndicators->side != 0 && pIndicators->entrySignal != 0 && pIndicators->side != pIndicators->entrySignal)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,Againt Side =%ld： skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->side, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss;
	double riskPNL;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);
	

	switch (pBase_Indicators->dailyTrend_Phase)
	{
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
	case RANGE_PHASE:
		//逆势空间对冲交易
		workoutExecutionTrend_Hedge(pParams, pIndicators, pBase_Indicators);		
		break;
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:					
		break;	
	}

	profitManagement(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}


static AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss;
	double riskPNL;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	struct tm timeInfo1;

	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int execution_tf, euro_index_rate, count;
	double ATR0_EURO = 10;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;

	switch (pBase_Indicators->dailyTrend_Phase)
	{
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:		
		workoutExecutionTrend_Pivot(pParams, pIndicators, pBase_Indicators,TRUE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal		
			workoutExecutionTrend_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, FALSE);
		pIndicators->splitTradeMode = 0;	

		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		workoutExecutionTrend_Pivot(pParams, pIndicators, pBase_Indicators, FALSE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal		
			workoutExecutionTrend_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal		
			workoutExecutionTrend_KongJian(pParams, pIndicators, pBase_Indicators);		
		
		break;
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
		workoutExecutionTrend_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators);
		break;
	case RANGE_PHASE:
		if ((int)parameter(AUTOBBS_RANGE) == 1)
		{
			if (pBase_Indicators->weeklyMATrend == RANGE)
				pIndicators->exitSignal = EXIT_ALL;				
			else
			{
				if (totalOpenOrders(pParams, BUY) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->dailyS) + pIndicators->adjust;
					modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0);
				}
				if (totalOpenOrders(pParams, SELL) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->dailyR) + pIndicators->adjust;
					modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0);
				}
			}
		}
		else
			pIndicators->exitSignal = EXIT_ALL;
		break;
	}

	/*
	如果到了17点，日内波幅都不超过80点，就离场日内的单子。
	*/
	if (timeInfo1.tm_hour >= 17)
	{
		execution_tf = (int)pParams->settings[TIMEFRAME];
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (17 - 1) * (60 / execution_tf) - 1;
		iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);

		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);  

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO, pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);

		if (ATR0_EURO < 8)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	}

	profitManagement(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}

//1. 在5M突破区间后，入场
//2. 但是如果不是第一单的话，需要在15分钟后，使用15 BBS Trend 检查
//3. 如果没有BBS Trend的支持，就close trade,先离场，等待下一次机会（这个时候，不能入场）
//4. 同时，使用total lose risk control < 0.6%
static void XAUUSD_Daily_Stop_Check(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime, openTime;
	struct tm timeInfo1, timeInfo2;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double diffMins;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	//这里简单处理，如果有单，都是使用0 的单子

	if (pParams->orderInfo[0].ticket != 0 && pParams->orderInfo[0].isOpen == TRUE)
	{

		// Must pass 15M
		openTime = pParams->orderInfo[0].openTime;
		safe_gmtime(&timeInfo2, pParams->orderInfo[0].openTime);

		diffMins = difftime(currentTime, openTime) / 60;

		if (diffMins == 15)
		{
			if (pParams->orderInfo[0].type == BUY && pIndicators->bbsTrend_secondary == -1)
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_BUY;
				}
			}
			if (pParams->orderInfo[0].type == SELL && pIndicators->bbsTrend_secondary == 1)
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_SELL;
				}
			}



		}
	}
}


static AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double closestR;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double down_gap = pIndicators->entryPrice - pBase_Indicators->pDailyLow;
	double up_gap = pBase_Indicators->pDailyHigh - pIndicators->entryPrice;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	//closeAllWithNegativeEasy(1, currentTime, 3);
	if (timeInfo1.tm_hour == 23 && timeInfo1.tm_min > 25)
	{
		pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 1;

	//Asia hour, risk 50%
	if (timeInfo1.tm_hour <= 8)
		pIndicators->risk = 0.5;

	if (pBase_Indicators->dailyTrend_Phase > 0)
	{
		pIndicators->executionTrend = -1;		
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& timeInfo1.tm_hour < 23
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
			pIndicators->entrySignal = -1;
		
		pIndicators->exitSignal = EXIT_BUY;

	}
	else if (pBase_Indicators->dailyTrend_Phase < 0)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		//逆势对冲交易
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index						
			&& timeInfo1.tm_hour < 23
			&&  !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;
	}
	else
	{
		//至少有1/3 空间,Sell
		if (up_gap <= pBase_Indicators->pDailyATR / 3)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
				&& timeInfo1.tm_hour < 23
				&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
				)
				pIndicators->entrySignal = -1;

			pIndicators->exitSignal = EXIT_BUY;
		}
		else if (down_gap <= pBase_Indicators->pDailyATR / 3)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pBase_Indicators->dailyS;
				pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

				//逆势对冲交易
				if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
					&& timeInfo1.tm_hour < 23
					&&  !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
					)
					pIndicators->entrySignal = 1;

				pIndicators->exitSignal = EXIT_SELL;
			}
	}
	return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double closestR;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(1, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 0;

	pIndicators->tradeMode = 1;
		
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
	{		
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);
		
		if (pParams->bidAsk.ask[0] - intradayLow > pBase_Indicators->dailyATR / 3 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
		{
			pIndicators->entrySignal = 1;
		}
		pIndicators->exitSignal = EXIT_SELL;		
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
	{
	
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);
		if (intradayHigh - pParams->bidAsk.bid[0] > pBase_Indicators->dailyATR / 3 &&!isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
		{			
			pIndicators->entrySignal = -1;
		}	
		pIndicators->exitSignal = EXIT_BUY;
	}
	
	

	
	return SUCCESS;
}


/*
一般是15M的BBS 突破交易系统
*/
static AsirikuyReturnCode workoutExecutionTrend_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators,BOOL ignored)
{
	double closestR;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(1, currentTime,3);
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 0;

	pIndicators->tradeMode = 1;
	
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index			
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust	
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR/3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			))
			pIndicators->entrySignal = 1;
		//Override the entry signal if the current daily low has been lower than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index			
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR/3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)))
			)
			pIndicators->entrySignal = -1;

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;
	}
	return SUCCESS;
}

/*
这是回调都pivot时候的入场法则。
*/
static AsirikuyReturnCode workoutExecutionTrend_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators,BOOL ignored)
{
	double closestR;
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double stopLoss;
	double preHigh = iHigh(B_SECONDARY_RATES, 1);
	double preLow = iLow(B_SECONDARY_RATES, 1);
	double preClose = iClose(B_SECONDARY_RATES, 1);
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(1, currentTime,3);
	
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 0;

	pIndicators->tradeMode = 1;

	//if (timeInfo1.tm_hour < 5)
	//	pIndicators->risk = 0.5;

	//满足多重通道当日动态的趋势分析
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0 ))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& preLow < pBase_Indicators->dailyPivot
			&& preClose >  pBase_Indicators->dailyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22			
			//&& bbsTrend == 1
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR/3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)))
			)
			pIndicators->entrySignal = 1;

		//Override the entry signal if the current daily low has been lower than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& preHigh > pBase_Indicators->dailyPivot
			&& preClose <  pBase_Indicators->dailyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22			
			//&& bbsTrend == -1
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR/3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			))
			pIndicators->entrySignal = -1;

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}


//日内BBS, 入场是15MBBS,止损是关键支撑位置 而且都是 S2/R2 only
//可以加入总风险控制，避免连续入场。
//还有必须 close 在日内23：45
static AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double closestR;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(1, currentTime,3);
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);	

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 0;

	pIndicators->tradeMode = 1;

	//满足多重通道当日动态的趋势分析
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_RETREAT_PHASE )
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666			
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)			
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE )
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666
			//&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)			
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

/*
不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 7 or 8?
4.最多交易3次，日内
5.10点之前，不交易
6.如果15MA too close, 例如 2，1H MA 方向不支持
*/
static BOOL XAUUSD_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate,euro_index_rate,execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);	

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	//execution_tf = pIndicators->executionRateTF;
	execution_tf = (int)pParams->settings[TIMEFRAME];

	if (timeInfo1.tm_hour < 10)
		return FALSE;
	
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = (10 - 1) * (60 / execution_tf) - 1;
	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

	if (timeInfo1.tm_hour >= 17) //17:00 进入美洲交易时段
	{
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (17 - 1) * (60 / execution_tf) - 1;
		iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);
	}

	if (fabs(pIndicators->asia_high - pIndicators->asia_low) >= 7.5)
		return FALSE; 

	if (iAtr(B_DAILY_RATES, 1, 1) >= 20)
		return FALSE;
	if (fabs(close_prev1 - close_prev2) >= 10)
		return FALSE;

	preHigh = iHigh(B_DAILY_RATES, 2);
	preLow = iLow(B_DAILY_RATES, 2);
	preClose = iClose(B_DAILY_RATES, 2);

	pivot = (preHigh + preLow + preClose) / 3;	
	S3 = preLow - 2 * (preHigh - pivot);
	R3 = preHigh + 2 * (pivot - preLow);
	
	if (close_prev1 > R3 || close_prev1 < S3)
		return FALSE;
	
	return TRUE;
}

static void XAUUSD_DayTrading_Entry(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double ATR0_EURO, double stopLoss, double Range)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	if (orderType == SELL)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

		pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
		pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
		if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
		{
			pIndicators->risk = pow(2, pIndicators->lossTimes);
			pIndicators->entrySignal = 1;
		}
		pIndicators->exitSignal = EXIT_SELL;
	}


	if (orderType == BUY)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
		pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
		pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
		if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
		{
			pIndicators->risk = pow(2, pIndicators->lossTimes);
			pIndicators->entrySignal = -1;
		}

		pIndicators->exitSignal = EXIT_BUY;
	}
}

/*
空间与BBS15的结合日内黄金交易法
亚洲时段： 
1- 10am

在有单子的情况下，在当日波幅小于 80p,必须使用空间法。

不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 7 or 8?
4.最多交易3次，日内
5.如果前一天到了S3 or R3

日内趋势：
MA15(50) > MA15(200): UP
MA15(50) < MA15(200): DOWN

入场:
1. 如果在10点的时候，顺势, 就马上入场
2. 如果10点的时候，逆势， 就等待到以下情况：
	2.1 如果5M的收盘突破亚洲时段的高低点，就入场
	2.2 直到顺势的15MBBS 突破， 入场

单量：
0.3 risk

出场：
日内23：30，必须离场

如果顺势： 必须5M突破日内的高低点，才出现信号
如果逆势： 到了BBS15的stoploss, 就出现信号。因为就变成顺势的BBS突破。

*/
static AsirikuyReturnCode workoutExecutionTrend_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double closestR;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow, primary_close_pre1 = iClose(B_PRIMARY_RATES, 1);
	double ATR0, Range, ATR0_EURO = 10;
	double close_prev1 = iClose(B_DAILY_RATES, 1);
	double openOrderHigh, openOrderLow, HalfPoint;

	int count;
	double adjust = 0.15;
	double stopLoss = pBase_Indicators->dailyATR*1.2;
	BOOL isOpen;
	OrderType side;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int MATrend_1H, noNewTradeSignal = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	Range = 8;
	stopLoss = 10;

	safe_timeString(timeString, currentTime);

	closeAllWithNegativeEasy(1, currentTime, 3);

	pIndicators->splitTradeMode = 2;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	pBase_Indicators->maTrend = getMATrend(2, B_SECONDARY_RATES, 1);

	//if ((int)parameter(AUTOBBS_IS_AUTO_MODE) && XAUUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
	if (XAUUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;


	count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count > 1)
		iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh, &intradayLow);

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	ATR0 = fabs(intradayHigh - intradayLow);


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf, asia_close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low, pIndicators->asia_close);

	if (timeInfo1.tm_hour >= 17)
	{
		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO,pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);
	}


	if (ATR0_EURO < Range
		//|| ATR0 > pBase_Indicators->pDailyMaxATR
		)
	{
		if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
			pIndicators->exitSignal = EXIT_ALL;

		//closeAllWithNegativeEasy(5, currentTime, 3);
		return SUCCESS;
	}

	// secondary rate is 15M , priarmy rate is 5M
	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)  //如果没有开仓, 就可以开仓
	{
		if (ATR0_EURO > Range && pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && timeInfo1.tm_hour < 22)//顺势
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
			pIndicators->entrySignal = 1;
		}
		else if (ATR0_EURO > Range && pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 && timeInfo1.tm_hour < 22)//顺势
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
			pIndicators->entrySignal = -1;
		}
		else //逆势，使用空间交易法,突破亚洲盘的高低点
		{
			if (ATR0_EURO > Range && primary_close_pre1 > pIndicators->asia_high && timeInfo1.tm_hour < 22)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
				pIndicators->entrySignal = 1;
			}

			if (ATR0_EURO > Range && primary_close_pre1 < pIndicators->asia_low && timeInfo1.tm_hour < 22)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
				pIndicators->entrySignal = -1;
			}
		}
	}
	else
	{

		side = getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
		if (side == SELL)
		{
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Side = SELL isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

			//小于80， 必须使用空间。
			//1. 在5M突破区间后，入场
			//2. 但是如果不是第一单的话，需要在15分钟后，使用15 BBS Trend 检查
			//3. 如果没有BBS Trend的支持，就close trade,先离场，等待下一次机会（这个时候，不能入场）

			if (ATR0 >= Range &&
				(pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && pIndicators->bbsIndex_secondary == shift1Index_secondary) //顺势，使用15BBS 买入BBS交易信号
				)
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
			}
			else
			{

				if (primary_close_pre1 - adjust > openOrderHigh)
				{
					XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
				}

				if (isOpen == FALSE)
				{
					if (primary_close_pre1 + adjust < openOrderLow)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
						if (pIndicators->entrySignal != 0)
							pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Enter a sell trade again.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					}
				}
			}

		}

		if (side == BUY)
		{
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Side = BUY isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

			if (ATR0 >= Range &&
				(pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 && pIndicators->bbsIndex_secondary == shift1Index_secondary) //顺势，使用15BBS 买入BBS交易信号
				)
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
			}
			else
			{
				if (primary_close_pre1 + adjust < openOrderLow)
				{
					XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
				}

				if (isOpen == FALSE)
				{
					if (primary_close_pre1 - adjust > openOrderHigh)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
						if (pIndicators->entrySignal != 0)
							pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Enter a buy trade again.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					}
				}
			}
		}

		if (pIndicators->entrySignal == 0 
			&& timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour <=17
			)
			XAUUSD_Daily_Stop_Check(pParams, pIndicators, pBase_Indicators);

		//在21點后，如果有單子，使用BBS15平仓
		if (pIndicators->entrySignal == 0 && timeInfo1.tm_hour >= 21)
		{
			if (isOpen == TRUE)
			{
				if ((pIndicators->bbsTrend_secondary == -1 && side == BUY)
					|| (pIndicators->bbsTrend_secondary == 1 && side == SELL)
					)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a trade on after 21H side = %ld.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,side);

					pIndicators->exitSignal = EXIT_ALL;
				}
			}
		}
	}


	MATrend_1H = getMATrend(4, B_HOURLY_RATES, 1);

	//if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 3)
	if (pIndicators->entrySignal != 0)
	{
		// too close 
		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1
			&& abs(pBase_Indicators->maTrend) <= 1 &&
			((pBase_Indicators->maTrend > 0 && MATrend_1H < 0)
			|| (pBase_Indicators->maTrend < 0 && MATrend_1H > 0)
			))
		{
			noNewTradeSignal = 1;
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, 15M Trend = %ld, 1H Trend = %ld,noNewTradeSignal=%ld",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, MATrend_1H, noNewTradeSignal);
			pIndicators->entrySignal = 0;
		}
	}

	

	//if (pIndicators->entrySignal == 0 && ATR0 > 8)
	//{
	//	HalfPoint = intradayHigh - ATR0 * 2 /3;

	//	if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
	//	{
	//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == BUY
	//			&& primary_close_pre1 < HalfPoint)
	//		{
	//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a buy order",
	//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString) ;
	//			pIndicators->exitSignal = EXIT_BUY;
	//		}
	//		

	//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == SELL
	//			&& primary_close_pre1 > HalfPoint)
	//		{
	//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a sell order",
	//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	//			pIndicators->exitSignal = EXIT_SELL;
	//		}
	//	}
	//}

	//if (pIndicators->entrySignal != 0 && pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity  > 0.01)
	//{
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Daily Loss=%lf, skip this entry signal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	return SUCCESS;
}



/*
这个策略是自用于辅助和对冲日内的autobbs 策略

1. 在日内短线信号出现逆势信号，而且有日内短单，就关闭，否则，不要开出日内逆势短单。等待下一个日内短线顺势信号。
2. 长单入场： issamedaysameprice
3. 短单入场： issamedaysamepricewithnotp
4. signal =1 (长），2（短）
*/
//static AsirikuyReturnCode workoutExecutionTrend__Auto_DailyHedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
//{
//	double closestR;
//	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
//	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
//	time_t currentTime;
//	struct tm timeInfo1;
//	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow, primary_close_pre1 = iClose(B_PRIMARY_RATES, 1);
//	double ATR0, Range;
//	double close_prev1 = iClose(B_DAILY_RATES, 1);
//	double openOrderHigh, openOrderLow;
//	int count;
//	double adjust = 0.15;
//	double stopLoss = pBase_Indicators->dailyATR*1.2;
//	char       timeString[MAX_TIME_STRING_SIZE] = "";
//	int MATrend_1H, noNewTradeSignal = 0;
//	OrderInfo * pDayOrder;
//
//	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
//	safe_gmtime(&timeInfo1, currentTime);
//	safe_timeString(timeString, currentTime);
//
//	Range = 8;
//	stopLoss = 10;
//
//	closeAllWithNegativeEasy(1, currentTime, 3);
//
//	pIndicators->splitTradeMode = 16;
//	pIndicators->risk = 1;
//	pIndicators->tpMode = 0;
//
//	pBase_Indicators->maTrend = getMATrend(2, B_SECONDARY_RATES, 1);
//
//	count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
//	if (count > 1)
//		iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh, &intradayLow);
//
//	intradayLow = min(close_prev1, intradayLow);
//	intradayHigh = max(close_prev1, intradayHigh);
//	ATR0 = fabs(intradayHigh - intradayLow);
//
//
//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);
//		
//
//	//如果有短线单
//	if (hasSameDayDayTradingOrderEasy(currentTime, pDayOrder, &openOrderHigh,&openOrderLow))
//	{
//		// SELL Order
//		if (pDayOrder->type == SELL)
//		{
//			if (ATR0 < Range)
//			{
//				if (primary_close_pre1 - adjust > openOrderHigh)
//				{				
//					closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
//				}
//			}
//			else
//			{
//				if ((pBase_Indicators->maTrend < 0 && primary_close_pre1 - adjust > openOrderHigh) //逆势，使用空间交易法 买入空间交易信号
//					|| (pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && pIndicators->bbsIndex_secondary == shift1Index_secondary) //顺势，使用15BBS 买入BBS交易信号
//					)
//				{
//					pIndicators->executionTrend = 1;
//					pIndicators->entryPrice = pParams->bidAsk.ask[0];
//					pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
//
//					pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
//					pIndicators->winTimes = getWinTimesInDayEasy(pIndicators->entryPrice, currentTime);
//					if (pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
//					{
//						pIndicators->risk = pow(2, pIndicators->lossTimes);
//						pIndicators->entrySignal = 1;
//					}
//					pIndicators->exitSignal = EXIT_SELL;
//				}
//			}
//		}
//
//	}
//
//	if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == BUY)
//	{
//		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf",
//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, openOrderHigh, openOrderLow);
//
//
//		if (ATR0 < Range) //小于80， 必须使用空间。
//		{
//			if (primary_close_pre1 + adjust < openOrderLow)
//			{
//				pIndicators->executionTrend = -1;
//				pIndicators->entryPrice = pParams->bidAsk.bid[0];
//				pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
//				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
//				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
//				if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
//				{
//					pIndicators->risk = pow(2, pIndicators->lossTimes);
//					pIndicators->entrySignal = -1;
//				}
//
//				pIndicators->exitSignal = EXIT_BUY;
//			}
//		}
//		else
//		{
//			if ((pBase_Indicators->maTrend > 0 && primary_close_pre1 + adjust < openOrderLow) //逆势，使用空间交易法 买入空间交易信号
//				|| (pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 && pIndicators->bbsIndex_secondary == shift1Index_secondary) //顺势，使用15BBS 买入BBS交易信号
//				)
//			{
//				pIndicators->executionTrend = -1;
//				pIndicators->entryPrice = pParams->bidAsk.bid[0];
//				pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
//				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
//				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
//				if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
//				{
//					pIndicators->risk = pow(2, pIndicators->lossTimes);
//					pIndicators->entrySignal = -1;
//				}
//
//				pIndicators->exitSignal = EXIT_BUY;
//			}
//		}
//	}
//	if (pIndicators->entrySignal == 0)
//		XAUUSD_Daily_Stop_Check(pParams, pIndicators, pBase_Indicators);
//
//
//	
//
//
//	MATrend_1H = getMATrend(4, B_HOURLY_RATES, 1);
//
//	//if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 3)
//	if (pIndicators->entrySignal != 0)
//	{
//		// too close 
//		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1
//			&& abs(pBase_Indicators->maTrend) <= 1 &&
//			((pBase_Indicators->maTrend > 0 && MATrend_1H < 0)
//			|| (pBase_Indicators->maTrend < 0 && MATrend_1H > 0)
//			))
//		{
//			noNewTradeSignal = 1;
//			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, 15M Trend = %ld, 1H Trend = %ld,noNewTradeSignal=%ld",
//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, MATrend_1H, noNewTradeSignal);
//			pIndicators->entrySignal = 0;
//		}
//	}
//
//	//if (pIndicators->entrySignal != 0 && pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity  > 0.01)
//	//{
//	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Daily Loss=%lf, skip this entry signal=%d",
//	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity, pIndicators->entrySignal);
//	//	pIndicators->entrySignal = 0;
//	//}
//
//	return SUCCESS;
//}


