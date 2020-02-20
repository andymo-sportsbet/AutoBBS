#include "Precompiled.h"
#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"

typedef enum additionalSettings_t
{
	MA_SHORT_PERIOD = ADDITIONAL_PARAM_1,
	MA_LONG_PERIOD = ADDITIONAL_PARAM_2,
} AdditionalSettings;

AsirikuyReturnCode runKelpie(StrategyParams* pParams)
{
	return runKelpie_BBS_1H_NoDailyTrend(pParams);
}

/*
Trend:
1H 200M
1H BBS(20)

Entry:
15M BBS(20)

Exit:
1H BBS(20) stop loss

Order Management:
Max: 10 orders
*/
AsirikuyReturnCode runKelpie_BBS_MultOrders(StrategyParams* pParams)
{
	double shortMA, longMA, atrDaily, atrPrimary, stopLoss, takeProfit;
	int trend1,trend2,trend3, dailyTrend = 0;
	double stopPrice1,stopPrice2,stopPrice3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
	double TwoDayHigh = 0, TwoDayLow = 0, OneDayHigh = 0, OneDayLow = 0, preHigh = 0, preLow = 0, preClose = 0, pivot = 0, S1, S2, S3, R1, R2, R3;
	int index1,index2,index3;

	double adjust1, adjust2, adjust3, risk = 1;

	atrDaily = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	atrPrimary = iAtr(PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	shortMA = iMA(3, HOURLY_RATES, (int)parameter(MA_SHORT_PERIOD), 1);
	longMA = iMA(3, HOURLY_RATES, (int)parameter(MA_LONG_PERIOD), 1);

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

	if (strcmp(timeString, "23/05/17 18:15") == 0)
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, "hit a point");

	iBBandStop(PRIMARY_RATES, 20, 2, &trend1, &stopPrice1, &index1);
	iBBandStop(HOURLY_RATES, 20, 2, &trend2, &stopPrice2, &index2);
	iBBandStop(FOURHOURLY_RATES, 20, 2, &trend3, &stopPrice3, &index3);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, trend1=%ld,trend2 = %ld,BBStopPrice1=%lf,BBStopPrice2=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, trend1, trend2, stopPrice1, stopPrice2);

	addValueToUI("ATR", atrDaily);
	addValueToUI("MA", shortMA);
	
	takeProfit = 0;

	//check daily trend on the new day.
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, close = %lf,shortMA=%lf,longMA=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index], shortMA, longMA);

	preHigh = iHigh(DAILY_RATES, 1);;
	preLow = iLow(DAILY_RATES, 1);
	preClose = iClose(DAILY_RATES, 1);
	pivot = (preHigh + preLow + preClose) / 3;
	S1 = 2 * pivot - preHigh;
	R1 = 2 * pivot - preLow;
	S2 = pivot - (R1 - S1);
	R2 = pivot - S1 + R1;
	S3 = pivot - (R2 - S2);
	R3 = pivot - S2 + R2;

	//Define uptrend if shortMA >longMA 10P
	adjust1 = 0.15; //10% daily ATR
	adjust2 = 0.1; //15M ATR
	adjust3 = 0.1;
	
	if (trend3 == 1 && trend2 == 1)
		dailyTrend = 1;
	if (trend3==-1 && trend2 == -1)
		dailyTrend = -1;

	if (dailyTrend == 1 && preClose - pivot <= adjust1)
		dailyTrend = 0;

	if (dailyTrend == -1 && preClose - pivot >= adjust1*-1)
		dailyTrend = 0;

	
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyTrend=%ld", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyTrend);

	//need to seperate the update and open 
	//EasyTrade lib is using 0 default execution time frame.
	if (trend1 == 1)
	{		
		if (dailyTrend == 1)
		{
			// Close all sell orders
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				closeAllShorts();
			}

			stopLoss = fabs(pParams->bidAsk.ask[0] - stopPrice2) + adjust2;	

			// The first order doesn't need to hit the exact signal point
			if (totalOpenOrders(pParams, BUY) == 0  || (totalOpenOrders(pParams, BUY) > 0 && index1 == shift1Index))
				return	openSingleLongEasy(takeProfit, stopLoss, 0);

		}		
	}
	if (trend2 == 1 )
	{
		stopLoss = fabs(pParams->bidAsk.ask[0] - stopPrice2) + adjust2;
		if (totalOpenOrders(pParams, BUY) > 0)
			modifyTradeEasy(BUY, -1, stopLoss, -1);
		return SUCCESS;
	}

	if (trend1 == -1)
	{
		if (dailyTrend == -1)
		{

			if (totalOpenOrders(pParams, BUY) > 0)
			{
				closeAllLongs();
			}
			stopLoss = fabs(stopPrice2 - pParams->bidAsk.bid[0]) + adjust2;
			if (totalOpenOrders(pParams, SELL) == 0 || (totalOpenOrders(pParams, SELL) > 0 && index1 == shift1Index))
				return openSingleShortEasy(takeProfit, stopLoss, 0);
		}		
	}

	if (trend2 == -1)
	{
		stopLoss = fabs(stopPrice2 - pParams->bidAsk.bid[0]) + adjust2;
		if (totalOpenOrders(pParams, SELL) > 0) // Only change stopLoss
			modifyTradeEasy(SELL, -1, stopLoss, -1);

		return SUCCESS;
	}

	return SUCCESS;

}

AsirikuyReturnCode runKelpie_Legacy(StrategyParams* pParams)
{
	double shortMA,longMA, atrDaily,atrPrimary, stopLoss, takeProfit;
	int trend, dailyTrend1 = 0;
	double stopPrice;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;	
	double gap = 0;
	double TwoDayHigh = 0, TwoDayLow = 0, OneDayHigh = 0, OneDayLow = 0, preHigh = 0, preLow = 0, preClose = 0, pivot = 0,S1,S2,S3,R1,R2,R3;
	int index;

	BOOL isAisaHour = FALSE;

	double adjust1, adjust2, adjust3, risk = 1;
	
	atrDaily = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	atrPrimary = iAtr(PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	shortMA = iMA(3, HOURLY_RATES, (int)parameter(MA_SHORT_PERIOD), 1);
	longMA = iMA(3, HOURLY_RATES, (int)parameter(MA_LONG_PERIOD), 1);

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

	if (hour() < 9) isAisaHour = TRUE;

	if (strcmp(timeString, "15/08/17 20:45") == 0)
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, "hit a point");

	iBBandStop(PRIMARY_RATES, 20, 2, &trend, &stopPrice, &index);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, trend=%ld,BBStopPrice=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, trend, stopPrice);
	
	addValueToUI("ATR", atrDaily);
	addValueToUI("MA", shortMA);

	stopLoss = atrDaily*parameter(SL_ATR_MULTIPLIER);
	takeProfit = atrDaily*parameter(SL_ATR_MULTIPLIER);

	//check daily trend on the new day.
	
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, close = %lf,shortMA=%lf,longMA=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index],shortMA, longMA);

	//Define uptrend if shortMA >longMA 10P
	adjust1 = atrDaily*0.1; //10% daily ATR
	adjust2 = atrPrimary; //15M ATR
	adjust3 = atrPrimary;
	//adjust1 = 0.10; //GBPJPY
	//adjust2 = 0.05;
	//adjust3 = 0.03;
	if (shortMA - longMA > adjust1 && pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] - longMA > adjust1)
		dailyTrend1 = 1;
	if (shortMA - longMA < adjust1*-1 && pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] - longMA < adjust1*-1)
		dailyTrend1 = -1;

	//Check Two day Highs and Lows
	OneDayHigh = iHigh(DAILY_RATES, 2);
	OneDayLow = iLow(DAILY_RATES, 2);
	TwoDayHigh = iHigh(DAILY_RATES, 3);
	TwoDayLow = iLow(DAILY_RATES, 3);
	preHigh = iHigh(DAILY_RATES, 1);;
	preLow = iLow(DAILY_RATES, 1);
	preClose = iClose(DAILY_RATES, 1);
	pivot = (preHigh + preLow + preClose) / 3;
	S1 = 2 * pivot - preHigh;
	R1 = 2 * pivot - preLow;
	S2 = pivot - (R1 - S1);
	R2 = pivot - S1 + R1;
	S3 = pivot - (R2 - S2);
	R3 = pivot - S2 + R2;

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, onedayHigh = %lf,onedayLow=%lf,twodayHigh=%lf,twodayLow=%lf,preHigh=%lf,preLow=%lf,preClose=%lf,pivot=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, OneDayHigh, OneDayLow, TwoDayHigh, TwoDayLow, preHigh, preLow, preClose, pivot);

	//Invalidate the trend
	if (dailyTrend1 == 1 && preClose < OneDayLow && preLow < TwoDayLow)
		dailyTrend1 = 0;

	//if (dailyTrend1 == 1 && preClose - pivot <= adjust1)
	//	dailyTrend1 = 0;

	if (dailyTrend1 == -1 && preClose > OneDayHigh && preHigh < TwoDayHigh)
		dailyTrend1 = 0;

	//if (dailyTrend1 == -1 && preClose - pivot >= adjust1*-1)
	//	dailyTrend1 = 0;

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyTrend=%ld", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyTrend1);

	//need to seperate the update and open 
	//EasyTrade lib is using 0 default execution time frame.
	if (trend == 1)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}

		if (dailyTrend1 == 1)
		{

			gap = fabs(pParams->bidAsk.ask[0] - stopPrice);
			//if (3 * gap > 0.5 *atrDaily)
			//	takeProfit = 3 * gap;

			if (gap < adjust3)
				return SUCCESS;

			//return openOrUpdateLongEasy(takeProfit, gap + adjust2, risk);

			//if (index == shift1Index && totalOpenOrders(pParams, BUY) == 0 && !isAisaHour)
			if (totalOpenOrders(pParams, BUY) == 0 )
			{
				if (pParams->bidAsk.bid[0] <= R3 - adjust1)
					return openOrUpdateLongEasy(takeProfit, gap + adjust2, risk);
			}
			else if (totalOpenOrders(pParams, BUY) >0)
				return modifyTradeEasy(BUY, -1, gap + adjust2, fabs(pParams->orderInfo[0].takeProfit - pParams->bidAsk.bid[0]));
		}
		
	}
	if (trend == -1)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
		if (dailyTrend1 == -1)
		{
			gap = fabs(stopPrice - pParams->bidAsk.bid[0]);
	/*		if (3 * gap > 0.5 *atrDaily)
				takeProfit = 3 * gap;*/
			if (gap < adjust3)
				return SUCCESS;
			//if (preClose - pivot >= adjust1*-1 )
			//	risk = 0.5;
			//return openOrUpdateShortEasy(takeProfit, gap + adjust2, risk);

			//if (index == shift1Index && totalOpenOrders(pParams, SELL) == 0 && !isAisaHour)
			if (totalOpenOrders(pParams, SELL) == 0 )
			{
				if (pParams->bidAsk.ask[0] >= S3 + adjust1)
					return openOrUpdateShortEasy(takeProfit, gap + adjust2, risk);
			}
			else  if (totalOpenOrders(pParams, SELL) >0)
				return modifyTradeEasy(SELL, -1, gap + adjust2, fabs(pParams->orderInfo[0].takeProfit - pParams->bidAsk.ask[0]));
		}
	}


}

AsirikuyReturnCode runKelpie_BBS(StrategyParams* pParams)
{
	double atrDaily, atrPrimary, stopLoss, takeProfit;
	int trend, dailyTrend = 0;
	double stopPrice;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
	int index;
	double adjust1, adjust2, adjust3, risk = 1;

	atrDaily = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	atrPrimary = iAtr(PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

	//if (hour() < 9) isAisaHour = TRUE;

	if (strcmp(timeString, "15/08/17 20:45") == 0)
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, "hit a point");

	iBBandStop(PRIMARY_RATES, 20, 2, &trend, &stopPrice, &index);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, trend=%ld,BBStopPrice=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, trend, stopPrice);

	addValueToUI("ATR", atrDaily);
	addValueToUI("MA", shortMA);

	//check daily trend on the new day.

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, close = %lf,shortMA=%lf,longMA=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index], shortMA, longMA);

	//Define uptrend if shortMA >longMA 10P
	adjust1 = 0.2; //10% daily ATR
	adjust2 = 0.1;
	adjust3 = 0.1;
	
	//need to seperate the update and open 
	//EasyTrade lib is using 0 default execution time frame.
	//if (trend == 1)
	//{
	//		gap = fabs(pParams->bidAsk.ask[0] - stopPrice);
	//		if (gap < adjust3)
	//			return SUCCESS;
	//		return openOrUpdateLongEasy(takeProfit, gap + adjust2, risk);		

	//}
	//if (trend == -1)
	//{
	//		gap = fabs(stopPrice - pParams->bidAsk.bid[0]);
	//		if (gap < adjust3)
	//			return SUCCESS;
	//		return openOrUpdateShortEasy(takeProfit, gap + adjust2, risk);					
	//}

	if (trend == 1)
	{		
			// Close all sell orders
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				closeAllShorts();
			}

			stopLoss = fabs(pParams->bidAsk.ask[0] - stopPrice);

			//Open 3 orders
			// 1. No TP
			// 2. 1:1
			// 3. 2:1
			if (index == shift1Index)
			{
				openSingleLongEasy(0, stopLoss+adjust2, 0.2);
				openSingleLongEasy(stopLoss + adjust2, stopLoss + adjust2, 0.1);
				openSingleLongEasy(2 * stopLoss + adjust2, stopLoss + adjust2, 0.1);
			}
			else
			{			
				if (totalOpenOrders(pParams, BUY) > 0)
					modifyTradeEasy(BUY, -1, stopLoss + adjust2, -1);
			}
			return SUCCESS;
	}	

	if (trend == -1)
	{		
			if (totalOpenOrders(pParams, BUY) > 0)
			{
				closeAllLongs();
			}

			stopLoss = fabs(stopPrice - pParams->bidAsk.bid[0]);

			if (index == shift1Index)
			{
				openSingleShortEasy(0, stopLoss + adjust2, 0.2);
				openSingleShortEasy(stopLoss + adjust2, stopLoss + adjust2, 0.1);
				openSingleShortEasy(2 * stopLoss + adjust2, stopLoss + adjust2, 0.1);
			}
			else
			{
				if (totalOpenOrders(pParams, SELL) > 0)
					modifyTradeEasy(SELL, -1, stopLoss + adjust2, -1);
			}	
			
	
	}

	

}