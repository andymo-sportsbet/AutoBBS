/*
* Trend Screening. Show support and resistance levels and work out the daily and weekly trend directions.
Primary rate: 1H
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

typedef enum trend_t
{
	UP =3,
	UP_NORMAL = 2,	
	UP_WEAK = 1,
	DOWN = -3,
	DOWN_NORMAL = -2,	
	DOWN_WEAK = -1,
	RANGE = 0
}trend;

typedef enum screeningRtesIndexes_t
{		
	S_HOURLY_RATES = 0,
	S_FOURHOURLY_RATES = 1,
	S_DAILY_RATES = 2,
	S_WEEKLY_RATES = 3
} ScreeningRatesIndexes;

typedef struct indicators_t
{	
	int dailyTrend;
	int daily3RulesTrend;
	int dailyMATrend;
	int dailyHLTrend;
	int weeklyTrend;
	int weekly3RulesTrend;
	int weeklyMATrend;
	int weeklyHLTrend;
	double dailyS;
	double dailyR;
	double dailyTP;	
	double dailyHigh;
	double dailyLow;
	double weeklyS;
	double weeklyR;
	double weeklyTP;
	double weeklyHigh;
	double weeklyLow;
	double monthlyHigh;
	double monthlyLow;
	double ma1H200M;
	double ma1H50M;
	double ma4H200M;
	double ma4H50M;

	double dailyATR;
	double weeklyATR;

	// Daily pivot point
	double dailyPivot;
	double dailyS1;
	double dailyR1;
	double dailyS2;
	double dailyR2;
	double dailyS3;
	double dailyR3;

	//Weekly pivot points
	double weeklyPivot;
	double weeklyS1;
	double weeklyR1;
	double weeklyS2;
	double weeklyR2;
	double weeklyS3;
	double weeklyR3;

} Indicators;

AsirikuyReturnCode runScreening(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runScreening() failed. pParams = NULL");
		return NULL_POINTER;
	}

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
	//if (strcmp(timeString, "08/09/17 21:00") == 0)
	//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, "hit a point");

	loadIndicators(pParams, &indicators);	
	
	setUIValues(pParams, &indicators);
	return SUCCESS;
}

//Get the previous 8 weeks high or low. 
static AsirikuyReturnCode loadMonthlyIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
	int shift0Index = pParams->ratesBuffers->rates[S_WEEKLY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[S_WEEKLY_RATES].time[shift0Index]);
	iSRLevels(pParams, pIndicators, S_WEEKLY_RATES, 8, &(pIndicators->monthlyHigh), &(pIndicators->monthlyLow));

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, 8weeksHigh=%lf, 8weeksLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->monthlyHigh, pIndicators->monthlyLow);
	return SUCCESS;
}

static AsirikuyReturnCode loadWeeklyIndicators(StrategyParams* pParams, Indicators* pIndicators)
{	
	int shift0Index = pParams->ratesBuffers->rates[S_WEEKLY_RATES].info.arraySize - 1;		
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[S_WEEKLY_RATES].time[shift0Index]);
		
	iTrend3Rules(pParams, pIndicators, S_WEEKLY_RATES, 2, &(pIndicators->weekly3RulesTrend));
	iTrend_HL(S_WEEKLY_RATES, &(pIndicators->weeklyHLTrend));
	iTrend_MA(pIndicators->weeklyATR, S_FOURHOURLY_RATES, &(pIndicators->weeklyMATrend));

	iSRLevels(pParams, pIndicators, S_WEEKLY_RATES, 2, &(pIndicators->weeklyHigh), &(pIndicators->weeklyLow));

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, weeklyHLTrend = %ld,weeklyMATrend=%ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyHLTrend, pIndicators->weeklyMATrend);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, weekly3RulesTrend = %ld,weeklyHigh=%lf, weeklyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weekly3RulesTrend, pIndicators->weeklyHigh, pIndicators->weeklyLow);

	workoutWeeklyTrend(pParams, pIndicators);
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, weeklyTrend=%ld, weeklySupport = %lf,weeklyResistance = %lf,weeklyTP=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyTrend, pIndicators->weeklyS, pIndicators->weeklyR, pIndicators->weeklyTP);

	return SUCCESS;

}

static AsirikuyReturnCode loadDailyIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
	int shift0Index = pParams->ratesBuffers->rates[S_DAILY_RATES].info.arraySize - 1;	
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	
	safe_timeString(timeString, pParams->ratesBuffers->rates[S_DAILY_RATES].time[shift0Index]);
		
	iTrend3Rules(pParams, pIndicators, S_DAILY_RATES, 2, &(pIndicators->daily3RulesTrend));
	iTrend_HL(S_DAILY_RATES, &(pIndicators->dailyHLTrend));
	iTrend_MA(pIndicators->dailyATR,S_HOURLY_RATES, &(pIndicators->dailyMATrend));

	iSRLevels(pParams, pIndicators, S_DAILY_RATES, 2, &(pIndicators->dailyHigh), &(pIndicators->dailyLow));

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyHLTrend = %ld,dailyMATrend=%ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyHLTrend, pIndicators->dailyMATrend);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, daily3RulesTrend = %ld,dailyHigh=%lf, dailyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->daily3RulesTrend, pIndicators->dailyHigh, pIndicators->dailyLow);

	workoutDailyTrend(pParams, pIndicators);
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyTrend=%ld, dailySupport = %lf,dailyResistance = %lf，dailyTP=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyTrend, pIndicators->dailyS, pIndicators->dailyR, pIndicators->dailyTP);

	return SUCCESS;

}



static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{	
	int shift0Index = pParams->ratesBuffers->rates[S_HOURLY_RATES].info.arraySize - 1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	safe_timeString(timeString, pParams->ratesBuffers->rates[S_HOURLY_RATES].time[shift0Index]);

	pIndicators->dailyATR = iAtr(S_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pIndicators->weeklyATR = iAtr(S_WEEKLY_RATES, 8, 1);

	//pIndicators->ma1H50M = iMA(3, S_DAILY_RATES, 2, 1);
	//pIndicators->ma1H200M = iMA(3, S_DAILY_RATES, 8, 1);
	//pIndicators->ma4H50M = iMA(3, S_DAILY_RATES, 12, 1);
	//pIndicators->ma4H200M = iMA(3, S_DAILY_RATES, 33, 1);

	pIndicators->ma1H50M = iMA(3, S_HOURLY_RATES, 50, 1);
	pIndicators->ma1H200M = iMA(3, S_HOURLY_RATES, 200, 1);
	pIndicators->ma4H50M = iMA(3, S_FOURHOURLY_RATES, 50, 1);
	pIndicators->ma4H200M = iMA(3, S_FOURHOURLY_RATES, 200, 1);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MA1H200M = %lf,MA4H200M=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->ma1H200M, pIndicators->ma4H200M);

	iPivot(S_DAILY_RATES, &(pIndicators->dailyPivot),
		&(pIndicators->dailyS1), &(pIndicators->dailyR1),
		&(pIndicators->dailyS2), &(pIndicators->dailyR2),
		&(pIndicators->dailyS3), &(pIndicators->dailyR3));


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyPivot = %lf,dailyS1=%lf, dailyR1 = %lf,dailyS2=%lf, dailyR2 = %lf,dailyS3=%lf, dailyR3 = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyPivot, pIndicators->dailyS1, pIndicators->dailyR1, pIndicators->dailyS2, pIndicators->dailyR2, pIndicators->dailyS3, pIndicators->dailyR3);

	iPivot(S_WEEKLY_RATES, &(pIndicators->weeklyPivot),
		&(pIndicators->weeklyS1), &(pIndicators->weeklyR1),
		&(pIndicators->weeklyS2), &(pIndicators->weeklyR2),
		&(pIndicators->weeklyS3), &(pIndicators->weeklyR3));

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, weeklyPivot = %lf,weeklyS1=%lf, weeklyR1 = %lf,weeklyS2=%lf, weeklyR2 = %lf,weeklyS3=%lf, weeklyR3 = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyPivot, pIndicators->weeklyS1, pIndicators->weeklyR1, pIndicators->weeklyS2, pIndicators->weeklyR2, pIndicators->weeklyS3, pIndicators->weeklyR3);

	loadMonthlyIndicators(pParams, pIndicators);

	loadWeeklyIndicators(pParams, pIndicators);
	loadDailyIndicators(pParams, pIndicators);
	

	return SUCCESS;
}


static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators)
{
	addValueToUI("DailyTrend", pIndicators->dailyTrend);
	addValueToUI("3 Days Rules", pIndicators->daily3RulesTrend);
	addValueToUI("WeeklyTrend", pIndicators->weeklyTrend);
	addValueToUI("3 Weeks Rules", pIndicators->weekly3RulesTrend);
	addValueToUI("DailyS", pIndicators->dailyS);
	addValueToUI("DailyR", pIndicators->dailyR);
	addValueToUI("WeeklyS", pIndicators->weeklyS);
	addValueToUI("WeeklyR", pIndicators->weeklyR);
	addValueToUI("DailyTp", pIndicators->dailyTP);
	addValueToUI("WeeklyTp", pIndicators->weeklyTP);
	

	return SUCCESS;
}

static AsirikuyReturnCode workoutDailyTrend(StrategyParams* pParams, Indicators* pIndicators)
{	
	double preLow = iLow(S_DAILY_RATES, 1);
	double preHigh = iHigh(S_DAILY_RATES, 1);
	double preClose = iClose(S_DAILY_RATES, 1);
	double currentLow = iLow(S_DAILY_RATES, 0);
	double currentHigh = iHigh(S_DAILY_RATES, 0);

	pIndicators->dailyTrend = pIndicators->daily3RulesTrend + pIndicators->dailyHLTrend + pIndicators->dailyMATrend;

	// 最后，修正趋势 和支撑阻力位置。
	if (pIndicators->dailyTrend == RANGE)
	{
		pIndicators->dailyS = pIndicators->dailyLow;
		if (pIndicators->dailyS == preLow && preClose-preLow < 0.5 *pIndicators->dailyATR) // shoudl look for the higher level resistance level
		{

			pIndicators->dailyS = iLow(S_WEEKLY_RATES, 0);
			//if (pIndicators->weeklyTrend < RANGE)
			//	pIndicators->dailyS = pIndicators->weeklyR;
			//else if (pIndicators->weeklyTrend > RANGE)
			//	pIndicators->dailyS = pIndicators->weeklyS;
			//else
			//	pIndicators->dailyS = pIndicators->weeklyLow;
		}


		pIndicators->dailyR = pIndicators->dailyHigh;
		if (pIndicators->dailyR == preHigh && preClose - preLow < 0.5 *pIndicators->dailyATR) // shoudl look for the higher level resistance level
		{
			pIndicators->dailyR = iHigh(S_WEEKLY_RATES, 0);

			//if (pIndicators->weeklyTrend < RANGE)
			//	pIndicators->dailyR = pIndicators->weeklyS;
			//else if (pIndicators->weeklyTrend > RANGE)
			//	pIndicators->dailyR = pIndicators->weeklyR;
			//else
			//	pIndicators->dailyR = pIndicators->weeklyHigh;
		}


		pIndicators->dailyTP = pIndicators->dailyR2;
	}
	else if (pIndicators->dailyTrend > RANGE)
	{
		pIndicators->dailyS = pIndicators->dailyLow;

		//if support level is too far from current price, like > 1.5 Daily ATR, 
		if (preClose - pIndicators->dailyS > 1.5 * pIndicators->dailyATR)
		{			
			pIndicators->dailyS = max(pIndicators->dailyS, pIndicators->ma1H200M);
			if (preClose - pIndicators->dailyS > 1.5 * pIndicators->dailyATR && preClose - preLow > pIndicators->dailyATR)
				pIndicators->dailyS = max(pIndicators->dailyS, preLow);

			if (preClose - pIndicators->dailyS > 1.5 * pIndicators->dailyATR && preClose - currentLow > pIndicators->dailyATR)
				pIndicators->dailyS = max(pIndicators->dailyS, currentLow);
		}

		pIndicators->dailyR = pIndicators->weeklyHigh;

		if (preClose - pIndicators->dailyHigh < 0)		
			pIndicators->dailyR = pIndicators->dailyHigh;
		else if (preClose - pIndicators->ma4H200M < 0)
			pIndicators->dailyR = min(pIndicators->weeklyHigh, pIndicators->ma4H200M);

		if (preClose - pIndicators->dailyR > 0)
			pIndicators->dailyR = pIndicators->monthlyHigh;

		// No resistance in two months.
		if (preClose - pIndicators->dailyR > 0)
			pIndicators->dailyR = -1;

		pIndicators->dailyTP = pIndicators->dailyR2;
		
	}
	else if (pIndicators->dailyTrend < RANGE)
	{
		pIndicators->dailyS = pIndicators->dailyHigh;


		if (pIndicators->dailyS - preClose > 1.5 * pIndicators->dailyATR)
		{
			pIndicators->dailyS = min(pIndicators->dailyS, pIndicators->ma1H200M);
			if (pIndicators->dailyS - preClose > 1.5 * pIndicators->dailyATR && preHigh - preClose > pIndicators->dailyATR)
				pIndicators->dailyS = min(pIndicators->dailyS, preHigh);

			if (pIndicators->dailyS - preClose > 1.5 * pIndicators->dailyATR && currentHigh - preClose > pIndicators->dailyATR)
				pIndicators->dailyS = min(pIndicators->dailyS, currentHigh);

		}

		pIndicators->dailyR = pIndicators->weeklyLow;
		if (preClose - pIndicators->dailyLow > 0)
			pIndicators->dailyR = pIndicators->dailyLow;
		else if (preClose - pIndicators->ma4H200M > 0)
			pIndicators->dailyR = max(pIndicators->weeklyLow, pIndicators->ma4H200M);
		
		if (preClose - pIndicators->dailyR < 0)
			pIndicators->dailyR = pIndicators->monthlyLow;

		// No resistance in two months.
		if (preClose - pIndicators->dailyR < 0)
			pIndicators->dailyR = -1;

		pIndicators->dailyTP = pIndicators->dailyS2;
	}

	return SUCCESS;
}

static AsirikuyReturnCode workoutWeeklyTrend(StrategyParams* pParams, Indicators* pIndicators)
{
	double preLow = iLow(S_WEEKLY_RATES, 1);
	double preHigh = iHigh(S_WEEKLY_RATES, 1);
	double preClose = iClose(S_WEEKLY_RATES, 1);
	double currentLow = iLow(S_WEEKLY_RATES, 0);
	double currentHigh = iHigh(S_WEEKLY_RATES, 0);

	// 最后，修正趋势 和支撑阻力位置。
	pIndicators->weeklyTrend = pIndicators->weekly3RulesTrend + pIndicators->weeklyHLTrend + pIndicators->weeklyMATrend;

	if (pIndicators->weeklyTrend == RANGE)
	{
		pIndicators->weeklyS = pIndicators->weeklyLow;
		pIndicators->weeklyR = pIndicators->weeklyHigh;
		pIndicators->weeklyTP = pIndicators->weeklyR2;
	}
	else if (pIndicators->weeklyTrend > RANGE)
	{

		pIndicators->weeklyS = pIndicators->ma4H200M;
		if (preClose - pIndicators->weeklyS > 1.5 * pIndicators->weeklyATR)
		{			
			pIndicators->weeklyS = max(pIndicators->weeklyS, pIndicators->weeklyLow);
			if (preClose - pIndicators->weeklyS > 1.5 * pIndicators->dailyATR && preClose - preLow > pIndicators->weeklyATR)
				pIndicators->weeklyS = max(pIndicators->weeklyS, preLow);
			if (preClose - pIndicators->weeklyS > 1.5 * pIndicators->dailyATR && preClose - currentLow > pIndicators->weeklyATR)
				pIndicators->weeklyS = max(pIndicators->weeklyS, currentLow);

		}

		pIndicators->weeklyR = pIndicators->monthlyHigh;
		// No resistance in two months.
		if (preClose - pIndicators->weeklyR > 0)
			pIndicators->weeklyR = -1;

		pIndicators->weeklyTP = pIndicators->weeklyR2;

	}
	else if (pIndicators->weeklyTrend < RANGE)
	{

		pIndicators->weeklyS = pIndicators->ma4H200M;

		if (pIndicators->weeklyS - preClose > 1.5 * pIndicators->weeklyATR)
		{
			pIndicators->weeklyS = min(pIndicators->weeklyS, pIndicators->weeklyHigh);
			if (pIndicators->weeklyS - preClose > 1.5 * pIndicators->dailyATR && preHigh - preClose > pIndicators->dailyATR)
				pIndicators->weeklyS = min(pIndicators->weeklyS, preHigh);
			if (pIndicators->weeklyS - preClose > 1.5 * pIndicators->dailyATR && currentHigh - preClose > pIndicators->dailyATR)
				pIndicators->weeklyS = min(pIndicators->weeklyS, currentHigh);

		}

		if (pIndicators->ma4H200M - preClose < pIndicators->weeklyATR)
		{
			if (pIndicators->weeklyHigh - preClose >= pIndicators->dailyATR)
				pIndicators->weeklyS = pIndicators->weeklyHigh;
		}
		

		pIndicators->weeklyR = pIndicators->monthlyLow;

		// No resistance in two months.
		if (preClose - pIndicators->weeklyR < 0)
			pIndicators->weeklyR = -1;

		pIndicators->weeklyTP = pIndicators->weeklyS2;
	}

	return SUCCESS;
}

static AsirikuyReturnCode iSRLevels(StrategyParams* pParams, Indicators* pIndicators, int ratesArrayIndex, int shift,double *pHigh,double *pLow)
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

//检查是否满足高点更高，低点更高，并且收盘也是更高，那就是UP
static AsirikuyReturnCode iTrend_HL(int ratesArrayIndex,int *trend)
{
	double preHigh1, preHigh2;
	double preLow1, preLow2;
	double preClose1, preClose2;

	*trend = RANGE;
	preHigh1 = iHigh(ratesArrayIndex, 1);
	preHigh2 = iHigh(ratesArrayIndex, 2);
	
	preClose1 = iClose(ratesArrayIndex, 1);
	preClose2 = iClose(ratesArrayIndex, 2);
	
	preLow1 = iLow(ratesArrayIndex, 1);
	preLow2 = iLow(ratesArrayIndex, 2);
	
	if (preHigh1 > preHigh2 &&
		preLow1 > preLow2 &&
		preClose1 > preClose2 )		
	{
		*trend = UP_WEAK;
	}
	
	if (preHigh1 < preHigh2 &&
		preLow1 < preLow2 &&
		preClose1 < preClose2)
	{
		*trend = DOWN_WEAK;
	}	
		
	return SUCCESS;
}

// Daily: 1H 200M , 50M
// Weekly: 1H 200M, 50M
static AsirikuyReturnCode iTrend_MA(double iATR,int ratesArrayIndex,int *trend)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(3, ratesArrayIndex, 50, 1);
	ma200M = iMA(3, ratesArrayIndex, 200, 1);
	if (ma50M - ma200M > 0.5 * iATR)
		*trend = UP_NORMAL;

	if (ma200M - ma50M > 0.5 * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

static AsirikuyReturnCode iTrend3Rules_LookBack(StrategyParams* pParams, Indicators* pIndicators, int ratesArrayIndex, int shift,int * pTrend)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int        outBegIdx, outNBElement;
	double	  boxHigh[100] = { 0 }, boxLow[100] = { 0 }, trend[100] = { 0 };
	int i = 0;

	retCode = TA_MIN(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	for (i = 0; i < outNBElement; i++)
	{
		// Check close price. 
		trend[i] = RANGE;
		if (boxHigh[i] == 0 || boxLow[i] == 0)
			break;

		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) > boxHigh[i]) //周规则，向上。 c3 > max(h1,h2) or c3 > max(c1,c2) && h3>max(h1,h2) && l3>max(l1,l2)
			trend[i] = UP;

		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) < boxLow[i]) //周规则，向上。
			trend[i] = DOWN;

		if (i - 1 >= 0 && trend[i - 1] == UP && trend[i] == RANGE)
			trend[i] = RANGE;

		if (i - 1 >= 0 && trend[i - 1] == DOWN && trend[i] == RANGE)
			trend[i] = RANGE;

		*pTrend = trend[i];
	}	

	return SUCCESS;
	
}

static AsirikuyReturnCode iTrend3Rules(StrategyParams* pParams, Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int       outBegIdx, outNBElement;
	double	  boxHigh, boxLow;	

	retCode = TA_MIN(shift1Index - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, &boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift1Index - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, &boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	*pTrend = RANGE;
	if (iClose(ratesArrayIndex, 1) > boxHigh) //周规则，向上。 c3 > max(h1,h2)
			*pTrend = UP;

	if (iClose(ratesArrayIndex, 1) < boxLow) //周规则，向上。
			*pTrend = DOWN;

	return SUCCESS;

}
