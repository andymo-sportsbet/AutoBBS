/**
 * @file
 * @brief     The Ayotl trading system.
 * @details   Ayotl is a portfolio of 2 trend following strategies trading on a daily timeframe.
 * @details   The first strategy uses long term Donchian Channel breakouts.
 * @details   The second strategy uses medium term Donchian Channel breakouts.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported Ayotl to the F4 framework)
 * @author    Franco (Assisted with porting Ayotl to the F4 framework)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#include "Precompiled.h"

#include <ta_libc.h>

#include "AsirikuyStrategies.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderManagement.h"
#include "OrderSignals.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "CriticalSection.h"

#define TESTING_MAX_POSITIONS              4
#define INSTRUMENTS_COUNT                  7
#define INSTANCE_COUNT                     14
#define MAX_TRADES_PER_INSTRUMENT          4
#define MAX_LONG_TRADES                    12
#define MAX_SHORT_TRADES                   12
#define MAX_STRONGLY_CORRELATED_TRADES     6
#define MAX_WEAKLY_CORRELATED_TRADES       10
#define STRONG_LOOSE_CORRELATION_THRESHOLD 0.8
#define RANGE_DAYS                         20
#define INDEX_NOT_FOUND                    -1

typedef enum additionalSettingsIndexes_t
{
  BREAKOUT_DAYS  = ADDITIONAL_PARAM_1,
  EXIT_DAYS      = ADDITIONAL_PARAM_2,
  TRADE_INTERVAL = ADDITIONAL_PARAM_3,
  INITIAL_ENTRY  = ADDITIONAL_PARAM_4,
} AdditionalSettingsIndexes;

typedef enum ayotlPositionResult_t
{
  AYOTL_POSITION_RESULT_UNDEFINED = -1,
  AYOTL_POSITION_RESULT_WIN       = 0,
  AYOTL_POSITION_RESULT_LOSS      = 1,
  AYOTL_POSITION_RESULT_BREAKEVEN = 2
} AyotlPositionResult;

typedef enum ayotlInstanceInfoIndexes_t
{
  AYOTL_INSTANCE_ID        = 0,
  AYOTL_INSTRUMENT_INDEX   = 1,
  AYOTL_TOTAL_LONG_TRADES  = 2,
  AYOTL_TOTAL_SHORT_TRADES = 3
} AyotlInstanceInfoIndexes;

/* Global variables are only accessed inside critical section to maintain thread safety. */
static int    gAyotlTotalInstances = 0;
static int    gAyotlInstanceInfo[INSTRUMENTS_COUNT * 2][4];
static int    gAyotlTotalInstruments = 0;
static char   gAyotlInstrumentNames[INSTRUMENTS_COUNT][MIN_SYMBOL_LENGTH] = {"", "", "", "", "", "", ""};
static int    gAyotlInstrumentTrades[INSTRUMENTS_COUNT] = {0, 0, 0, 0, 0, 0, 0};
static double gAyotlInstrumentMonthlyPriceRange[INSTRUMENTS_COUNT] = {0, 0, 0, 0, 0, 0, 0};
static double gAyotlCorrelations[INSTRUMENTS_COUNT][INSTRUMENTS_COUNT];

int getInstanceIndex(int instanceId)
{
  int i;
  for(i = 0; i < gAyotlTotalInstances; i++)
  {
    if(instanceId == gAyotlInstanceInfo[i][AYOTL_INSTANCE_ID])
    {
      return i;
    }
  }

  return INDEX_NOT_FOUND;
}

int getInstrumentIndex(char instrumentName[MIN_SYMBOL_LENGTH])
{
  int i;
  for(i = 0; i < gAyotlTotalInstruments; i++)
  {
    if(strcmp(instrumentName, gAyotlInstrumentNames[i]) == 0)
    {
      return i;
    }
  }

  return INDEX_NOT_FOUND;
}

void calculateTradeTotals(int instanceIndex, int* totalLongTrades, int* totalShortTrades)
{
  int i;
  *totalLongTrades  = 0;
  *totalShortTrades = 0;
  gAyotlInstrumentTrades[gAyotlInstanceInfo[instanceIndex][AYOTL_INSTRUMENT_INDEX]] = 0;

  for(i = 0; i < gAyotlTotalInstances; i++)
  {
    *totalLongTrades  += gAyotlInstanceInfo[i][AYOTL_TOTAL_LONG_TRADES];
    *totalShortTrades += gAyotlInstanceInfo[i][AYOTL_TOTAL_SHORT_TRADES];

    if(gAyotlInstanceInfo[i][AYOTL_INSTRUMENT_INDEX] == gAyotlInstanceInfo[instanceIndex][AYOTL_INSTRUMENT_INDEX])
    {
      gAyotlInstrumentTrades[gAyotlInstanceInfo[instanceIndex][AYOTL_INSTRUMENT_INDEX]] += (gAyotlInstanceInfo[i][AYOTL_TOTAL_LONG_TRADES] + gAyotlInstanceInfo[i][AYOTL_TOTAL_SHORT_TRADES]);
    }
  }
}

void calculateInstrumentCorrelations()
{
  int i, j;
	for(i = 0; i < gAyotlTotalInstruments; i++)
	{
		for(j = 0; j < gAyotlTotalInstruments; j++)
		{
			if(gAyotlInstrumentMonthlyPriceRange[i] > gAyotlInstrumentMonthlyPriceRange[j])
			{
		    gAyotlCorrelations[i][j] = gAyotlInstrumentMonthlyPriceRange[j] / gAyotlInstrumentMonthlyPriceRange[i];
			}
			else if(gAyotlInstrumentMonthlyPriceRange[i] <= gAyotlInstrumentMonthlyPriceRange[j])
			{
			  gAyotlCorrelations[i][j] = gAyotlInstrumentMonthlyPriceRange[i] / gAyotlInstrumentMonthlyPriceRange[j];
			}
		}
	}
}

BOOL isTooManyCorrelatedTrades(int instrumentIndex)
{
  int i, stronglyCorrelatedTradesCount = 0, weaklyCorrelatedTradesCount = 0;

  for(i = 0; i < gAyotlTotalInstruments; i++) 
  {
    if((fabs(gAyotlCorrelations[instrumentIndex][i]) >= STRONG_LOOSE_CORRELATION_THRESHOLD)  && (gAyotlInstrumentTrades[i] > 0))
    {
      stronglyCorrelatedTradesCount += gAyotlInstrumentTrades[i];
    }

    if((fabs(gAyotlCorrelations[instrumentIndex][i]) < STRONG_LOOSE_CORRELATION_THRESHOLD)  && (gAyotlInstrumentTrades[i] > 0))
    {
      weaklyCorrelatedTradesCount += gAyotlInstrumentTrades[i];
    }
  }

  if((stronglyCorrelatedTradesCount > MAX_STRONGLY_CORRELATED_TRADES) || (weaklyCorrelatedTradesCount > MAX_WEAKLY_CORRELATED_TRADES))
  {
    return TRUE;
  }
  
  return FALSE;
}

BOOL canOpenLong(int instanceTradesCount, int longTradesQuota)
{
	if((instanceTradesCount >= MAX_TRADES_PER_INSTRUMENT) || (longTradesQuota <= 0))
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL canOpenShort(int instanceTradesCount, int shortTradesQuota)
{
	if((instanceTradesCount >= MAX_TRADES_PER_INSTRUMENT) || (shortTradesQuota  <= 0))
	{
		return FALSE;
	}
	
	return TRUE;
}

AyotlPositionResult retrievePreviousPositionResult(StrategyParams* pParams)
{
  int i;
  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
  {
    if((pParams->orderInfo[i].isOpen == FALSE))
    {
      if(fabs(pParams->orderInfo[i].openPrice - pParams->orderInfo[i].closePrice) <= DBL_EPSILON)
      {
        return AYOTL_POSITION_RESULT_BREAKEVEN;
      }

      if(pParams->orderInfo[i].type == BUY)
      {
        if(pParams->orderInfo[i].closePrice < pParams->orderInfo[i].openPrice)
        {
          return AYOTL_POSITION_RESULT_LOSS;
        }

        if(pParams->orderInfo[i].closePrice > pParams->orderInfo[i].openPrice)
        {
          return AYOTL_POSITION_RESULT_WIN;
        }
      }

      if(pParams->orderInfo[i].type == SELL)
      {
        if(pParams->orderInfo[i].closePrice > pParams->orderInfo[i].openPrice)
        {
          return AYOTL_POSITION_RESULT_LOSS;
        }

        if(pParams->orderInfo[i].closePrice < pParams->orderInfo[i].openPrice)
        {
          return AYOTL_POSITION_RESULT_WIN;
        }
      }
    }
  }

  return AYOTL_POSITION_RESULT_UNDEFINED;
}

double calculateLongEntryPrice(StrategyParams* pParams, AyotlPositionResult previousPositionResult, double initialEntryDaysHigh, double breakoutDaysHigh, double atr)
{
  if((pParams->orderInfo[0].type == BUY) && pParams->orderInfo[0].isOpen)
  {
    return (pParams->orderInfo[0].openPrice + (pParams->settings[SL_ATR_MULTIPLIER] * atr));
  }
  else
  {
    if(previousPositionResult == AYOTL_POSITION_RESULT_LOSS)
    {
			return breakoutDaysHigh;
    }
    else
    {
      return initialEntryDaysHigh;
	  }
  }
}

double calculateShortEntryPrice(StrategyParams* pParams, AyotlPositionResult previousPositionResult, double initialEntryDaysLow, double breakoutDaysLow, double atr)
{
  if((pParams->orderInfo[0].type == SELL) && pParams->orderInfo[0].isOpen)
  {
    return (pParams->orderInfo[0].openPrice - (pParams->settings[SL_ATR_MULTIPLIER] * atr));
  }
  else
  {
    if(previousPositionResult == AYOTL_POSITION_RESULT_LOSS)
    {
			return breakoutDaysLow;
    }
    else
    {
      return initialEntryDaysLow;
	  }
  }
}

AsirikuyReturnCode runAyotl(StrategyParams* pParams)
{
  const int  MIN_MAX_INDEX_SHIFT = 1;
  TA_RetCode retCode;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atr, longEntryPrice, shortEntryPrice;
  int        tradingSignals = 0, entryDaysLowIndex, entryDaysHighIndex, breakoutDaysLowIndex, breakoutDaysHighIndex, exitDaysLowIndex, exitDaysHighIndex, rangeDaysLowIndex, rangeDaysHighIndex, outBegIdx, outNBElement;
  int        totalLongTrades, totalShortTrades, instanceIndex, instrumentIndex, shift0Index, shift1Index;
  BOOL       tooManyCorrelatedTrades, tooManyLongTrades, tooManyShortTrades, tooManyTradesOnInstrument;
  Rates*     rates;
  AyotlPositionResult previousPositionResult;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runAyotl() failed. pParams = NULL");
    return NULL_POINTER;
  }

  rates = &pParams->ratesBuffers->rates[0];
  shift0Index = rates->info.arraySize - 1;
  shift1Index = rates->info.arraySize - 2;

  /* Load indicators */
  retCode  = TA_ATR(shift1Index, shift1Index, rates->high, rates->low, rates->close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("runAyotl()", retCode);
    return TA_LIB_ERROR;
  }

  minIndex(rates->low,  rates->info.arraySize, (int)pParams->settings[INITIAL_ENTRY] - 1, MIN_MAX_INDEX_SHIFT, &entryDaysLowIndex);
  maxIndex(rates->high, rates->info.arraySize, (int)pParams->settings[INITIAL_ENTRY] - 1, MIN_MAX_INDEX_SHIFT, &entryDaysHighIndex);
  minIndex(rates->low,  rates->info.arraySize, (int)pParams->settings[BREAKOUT_DAYS] - 1, MIN_MAX_INDEX_SHIFT, &breakoutDaysLowIndex);
  maxIndex(rates->high, rates->info.arraySize, (int)pParams->settings[BREAKOUT_DAYS] - 1, MIN_MAX_INDEX_SHIFT, &breakoutDaysHighIndex);
  minIndex(rates->low,  rates->info.arraySize, (int)pParams->settings[EXIT_DAYS]     - 1, MIN_MAX_INDEX_SHIFT, &exitDaysLowIndex);
  maxIndex(rates->high, rates->info.arraySize, (int)pParams->settings[EXIT_DAYS]     - 1, MIN_MAX_INDEX_SHIFT, &exitDaysHighIndex);
  minIndex(rates->low,  rates->info.arraySize, (int)pParams->settings[RANGE_DAYS]    - 1, MIN_MAX_INDEX_SHIFT, &rangeDaysLowIndex);
  maxIndex(rates->high, rates->info.arraySize, (int)pParams->settings[RANGE_DAYS]    - 1, MIN_MAX_INDEX_SHIFT, &rangeDaysHighIndex);

  previousPositionResult = retrievePreviousPositionResult(pParams);
  longEntryPrice         = calculateLongEntryPrice(pParams, previousPositionResult, rates->high[entryDaysHighIndex], rates->high[breakoutDaysHighIndex], atr);
  shortEntryPrice        = calculateShortEntryPrice(pParams, previousPositionResult, rates->low[entryDaysLowIndex], rates->low[breakoutDaysLowIndex], atr);

  enterCriticalSection();
  {
    instrumentIndex = getInstrumentIndex(pParams->tradeSymbol);
    if(instrumentIndex == INDEX_NOT_FOUND)
    {
      strcpy(gAyotlInstrumentNames[gAyotlTotalInstruments], pParams->tradeSymbol);
      instrumentIndex = gAyotlTotalInstruments;
      gAyotlTotalInstruments++;
    }
    instanceIndex = getInstanceIndex((int)pParams->settings[STRATEGY_INSTANCE_ID]);
    if(instanceIndex == INDEX_NOT_FOUND)
    {
      gAyotlInstanceInfo[gAyotlTotalInstances][AYOTL_INSTANCE_ID]        = (int)pParams->settings[STRATEGY_INSTANCE_ID];
      gAyotlInstanceInfo[gAyotlTotalInstances][AYOTL_INSTRUMENT_INDEX]   = instrumentIndex;
      gAyotlInstanceInfo[gAyotlTotalInstances][AYOTL_TOTAL_LONG_TRADES]  = totalOpenOrders(pParams, BUY);
      gAyotlInstanceInfo[gAyotlTotalInstances][AYOTL_TOTAL_SHORT_TRADES] = totalOpenOrders(pParams, SELL);
      instanceIndex = gAyotlTotalInstances;
      gAyotlTotalInstances++;
    }
    calculateTradeTotals(instanceIndex, &totalLongTrades, &totalShortTrades);
    tooManyLongTrades = (totalLongTrades >= MAX_LONG_TRADES);
    tooManyShortTrades = (totalShortTrades >= MAX_SHORT_TRADES);
    tooManyTradesOnInstrument = (gAyotlInstrumentTrades[instrumentIndex] >= MAX_TRADES_PER_INSTRUMENT);
    gAyotlInstrumentMonthlyPriceRange[instrumentIndex] = rates->high[rangeDaysHighIndex] - rates->low[rangeDaysLowIndex];
    calculateInstrumentCorrelations();
    tooManyCorrelatedTrades = isTooManyCorrelatedTrades(instrumentIndex);
  }
  leaveCriticalSection();

  /* Always set the SL so that orders can be modified on later ticks. */
  pParams->results[0].brokerSL = atr * pParams->settings[SL_ATR_MULTIPLIER];
  
  safe_timeString(timeString, pParams->ratesBuffers->rates[0].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Ayotl InstanceID = %d, BarTime = %s, ATR = %lf, EntryDaysLowIndex = %d, EntryDaysHighIndex = %d, BreakoutDaysLowIndex = %d, BreakoutDaysHighIndex = %d, ExitDaysLowIndex = %d, ExitDaysHighIndex = %d, RangeDaysLowIndex = %d, RangeDaysHighIndex = %d, TotalLongTrades = %d, TotalShortTrades = %d, TooManyCorrelatedTrades = %d, TooManyLongTrades = %d, TooManyShortTrades = %d, TooManyTradesOnInstrument = %d, PreviousPositionResult = %d"
    , (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, entryDaysLowIndex, entryDaysHighIndex, breakoutDaysLowIndex, breakoutDaysHighIndex, exitDaysLowIndex, exitDaysHighIndex, rangeDaysLowIndex, rangeDaysHighIndex, totalLongTrades, totalShortTrades, tooManyCorrelatedTrades, tooManyLongTrades, tooManyShortTrades, tooManyTradesOnInstrument, previousPositionResult);
  
  /* Generate trading signals */
  if((totalOpenOrders(pParams, BUY) > 0) && (rates->low[exitDaysLowIndex] >= pParams->bidAsk.bid[0]))
  {
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Ayotl InstanceID = %d, BarTime = %s, TradeSignal(Exit criteria) : Exit BUY", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	  addOrderExitSignal(BUY, pParams, &tradingSignals);
  }
  if((totalOpenOrders(pParams, SELL) > 0) && (rates->high[exitDaysHighIndex] <= pParams->bidAsk.ask[0]))
  {
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Ayotl InstanceID = %d, BarTime = %s, TradeSignal(Exit criteria) : Exit SELL", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	  addOrderExitSignal(SELL, pParams, &tradingSignals);
  }

  if((pParams->bidAsk.ask[0] > longEntryPrice) && !tooManyTradesOnInstrument && !tooManyCorrelatedTrades && !tooManyLongTrades)
  {
    pParams->results[0].entryPrice = pParams->bidAsk.ask[0];
    pParams->results[0].lots       = calculateOrderSize(pParams, BUY, pParams->bidAsk.ask[0], pParams->results[0].brokerSL);
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Ayotl InstanceID = %d, BarTime = %s, TradeSignal(Entry criteria): Enter BUY, EntryPrice = %lf, Lots = %lf, SL = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->results[0].entryPrice, pParams->results[0].lots, pParams->results[0].brokerSL);
    return addOrderEntrySignal(BUY, pParams, &tradingSignals);
  }
  else if((pParams->bidAsk.bid[0] < shortEntryPrice) && !tooManyTradesOnInstrument && !tooManyCorrelatedTrades && !tooManyShortTrades)
  {
    pParams->results[0].entryPrice = pParams->bidAsk.bid[0];
    pParams->results[0].lots       = calculateOrderSize(pParams, SELL, pParams->bidAsk.bid[0], pParams->results[0].brokerSL);
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Ayotl InstanceID = %d, BarTime = %s, TradeSignal(Entry criteria): Enter SELL, EntryPrice = %lf, Lots = %lf, SL = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->results[0].entryPrice, pParams->results[0].lots, pParams->results[0].brokerSL);
    return addOrderEntrySignal(SELL, pParams, &tradingSignals);
  }

	return SUCCESS;
}