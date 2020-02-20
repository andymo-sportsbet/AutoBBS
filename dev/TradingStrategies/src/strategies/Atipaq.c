/**
 * @file
 * @brief     The Atipaq trading system.
 * @details   Atipaq is a box breakout or fading strategy using hourly or daily timeframes.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Morgan Doel (Ported Atipaq to the F4 framework and merged Atipaq with Kutichiy)
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

#include "StrategyUserInterface.h"
#include "AsirikuyStrategies.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "CriticalSection.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP TRUE

typedef enum additionalSettings_t
{
  ENTRY_LOGIC          = ADDITIONAL_PARAM_1, /* 0 = Trade breakouts. 1 = Fade breakouts. */
  SIGNAL_DAY           = ADDITIONAL_PARAM_2, /* Hourly - Only trade on the specified day of the week( 1 - 5), or every day(-1). Daily - This parameter is not used. */
  BOX_END_TIME         = ADDITIONAL_PARAM_3, /* Hourly - The time of the day (0 - 23). Daily - The day of the week (1 - 5) */
  BOX_LENGTH           = ADDITIONAL_PARAM_4, /* Number of bars to count back to set as the start of the box. */
  MOVE_SL_BOX_MULTIPLE = ADDITIONAL_PARAM_5, /* Distance as box multiple to move the stop-loss. */
  BUFFER_BOX_MULTIPLE  = ADDITIONAL_PARAM_6, /* Distance as box multiple from high/low to enter trades. */
  PROFIT_BOX_MULTIPLE  = ADDITIONAL_PARAM_7, /* Profit target as a box multiple. */
  MIN_BOX_SIZE_ATR     = ADDITIONAL_PARAM_8, /* Don't enter trades if the box size is smaller than this multiple of the ATR. */
  MAX_BOX_SIZE_ATR     = ADDITIONAL_PARAM_9  /* Don't enter trades if the box size is larger than this multiple of the ATR. */
} AdditionalSettings;

typedef enum ratesIndexes_t
{
  PRIMARY_RATES = 0,
  DAILY_RATES   = 1
} RatesIndexes;

typedef enum entryLogic_t
{
  TRADE_BOX_BREAKOUTS = 0,
  FADE_BOX_BREAKOUTS  = 1
} EntryLogic;


static AsirikuyReturnCode validateAtipaqParameters(StrategyParams* pParams)
{
  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"validateParameters() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(((int)pParams->settings[TIMEFRAME] != MINUTES_PER_HOUR) && ((int)pParams->settings[TIMEFRAME] != MINUTES_PER_DAY))
  {
    pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"validateParameters() failed. Instance Id = %d, STRATEGY_TIMEFRAME = %d. Please set STRATEGY_TIMEFRAME to 60 or 1440 only.", (int)pParams->settings[STRATEGY_INSTANCE_ID], (int)pParams->settings[TIMEFRAME]);
    return INVALID_PARAMETER;
  }

  if(((int)pParams->settings[ENTRY_LOGIC] != TRADE_BOX_BREAKOUTS) && ((int)pParams->settings[ENTRY_LOGIC] != FADE_BOX_BREAKOUTS))
  {
    pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"validateParameters() failed. Instance Id = %d, ENTRY_LOGIC = %d. Please set ENTRY_LOGIC to 0 or 1 only.", (int)pParams->settings[STRATEGY_INSTANCE_ID], (int)pParams->settings[ENTRY_LOGIC]);
    return INVALID_PARAMETER;
  }

  if(((int)pParams->settings[TIMEFRAME] == MINUTES_PER_HOUR) &&  (((int)pParams->settings[BOX_END_TIME] < 0) || ((int)pParams->settings[BOX_END_TIME] > 23)))
  {
    pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"validateParameters() failed. Instance Id = %d, BOX_END_TIME = %d. When using an hourly timeframe please set BOX_END_TIME to an integer from 0 to 23", (int)pParams->settings[STRATEGY_INSTANCE_ID], (int)pParams->settings[BOX_END_TIME]);
    return INVALID_PARAMETER;
  }

  if(((int)pParams->settings[TIMEFRAME] == MINUTES_PER_DAY) &&  (((int)pParams->settings[BOX_END_TIME] < 1) || ((int)pParams->settings[BOX_END_TIME] > 5)))
  {
    pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"validateParameters() failed. Instance Id = %d, BOX_END_TIME = %d. When using a daily timeframe please set BOX_END_TIME to an integer from 1 to 5", (int)pParams->settings[STRATEGY_INSTANCE_ID], (int)pParams->settings[BOX_END_TIME]);
    return INVALID_PARAMETER;
  }

  return SUCCESS;
}

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, double atr, double boxLow, double boxHigh, double boxSize, double boxBuffer)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  double userInterfaceValues[TOTAL_UI_VALUES];
  char   *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
    "ATR",
    "Box Low",
    "Box High",
    "Box Size",
    "Box Buffer",
    "Box end hour/day",
    "",
    "",
    "",
    ""};

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"setUIValues() failed. pParams = NULL");
    return NULL_POINTER;
  }

  userInterfaceValues[0] = atr;
  userInterfaceValues[1] = boxLow;
  userInterfaceValues[2] = boxHigh;
  userInterfaceValues[3] = boxSize;
  userInterfaceValues[4] = boxBuffer;
  userInterfaceValues[5] = pParams->settings[BOX_END_TIME];
  userInterfaceValues[6] = 0;
  userInterfaceValues[7] = 0;
  userInterfaceValues[8] = 0;
  userInterfaceValues[9] = 0;

	return saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, TOTAL_UI_VALUES, (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, double* pATR, double* pBoxSize, double* pBoxHigh, double* pBoxLow, double* pBoxBuffer, time_t* pMarketAnalysisTime)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  TA_RetCode retCode;
  struct tm  marketAnalysisTimeInfo;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  int        i, indicatorShift = -1, outBegIdx, outNBElement;
  int        primaryShift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
  int        dailyShift1Index   = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pATR == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pATR = NULL");
    return NULL_POINTER;
  }

  if(pBoxSize == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pBoxSize = NULL");
    return NULL_POINTER;
  }

  if(pBoxHigh == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pBoxHigh = NULL");
    return NULL_POINTER;
  }

  if(pBoxLow == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pBoxLow = NULL");
    return NULL_POINTER;
  }

  if(pBoxBuffer == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pBoxBuffer = NULL");
    return NULL_POINTER;
  }

  if(pMarketAnalysisTime == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pMarketAnalysisTime = NULL");
    return NULL_POINTER;
  }

  retCode = TA_ATR(dailyShift1Index, dailyShift1Index, pParams->ratesBuffers->rates[DAILY_RATES].high, pParams->ratesBuffers->rates[DAILY_RATES].low, pParams->ratesBuffers->rates[DAILY_RATES].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, pATR);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  if(pParams->settings[TIMEFRAME] == MINUTES_PER_HOUR)
  {
    for(i = primaryShift0Index - 1; i > (int)pParams->settings[BOX_LENGTH]; i--)
    {
      safe_gmtime(&marketAnalysisTimeInfo, pParams->ratesBuffers->rates[PRIMARY_RATES].time[i]);
      if((marketAnalysisTimeInfo.tm_hour == (int)pParams->settings[BOX_END_TIME]) &&
		 ((marketAnalysisTimeInfo.tm_wday == (int)pParams->settings[SIGNAL_DAY]) || 
		 ((int)pParams->settings[SIGNAL_DAY] == EVERY_DAY)) )
      {
        indicatorShift = primaryShift0Index - i;
#if defined _WIN32 || defined _WIN64
        *pMarketAnalysisTime = _mkgmtime(&marketAnalysisTimeInfo);
#else
        *pMarketAnalysisTime = timegm(&marketAnalysisTimeInfo);
#endif
        break;
      }
    }
  }
  else
  {
    for(i = primaryShift0Index - 1; i > (int)pParams->settings[BOX_LENGTH]; i--)
    {
      safe_gmtime(&marketAnalysisTimeInfo, pParams->ratesBuffers->rates[PRIMARY_RATES].time[i]);
      if(marketAnalysisTimeInfo.tm_wday == (int)pParams->settings[BOX_END_TIME])
      {
        indicatorShift = primaryShift0Index - i;
#if defined _WIN32 || defined _WIN64
        *pMarketAnalysisTime = _mkgmtime(&marketAnalysisTimeInfo);
#else
        *pMarketAnalysisTime = timegm(&marketAnalysisTimeInfo);
#endif
        break;
      }
    }
  }

  retCode = TA_MIN(primaryShift0Index-indicatorShift, primaryShift0Index-indicatorShift, pParams->ratesBuffers->rates[PRIMARY_RATES].low, (int)pParams->settings[BOX_LENGTH], &outBegIdx, &outNBElement , pBoxLow);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_MIN()", retCode);
  }
  
  retCode = TA_MAX(primaryShift0Index-indicatorShift, primaryShift0Index-indicatorShift, pParams->ratesBuffers->rates[PRIMARY_RATES].high, (int)pParams->settings[BOX_LENGTH], &outBegIdx, &outNBElement, pBoxHigh);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_MAX()", retCode);
  }

  *pBoxSize     = *pBoxHigh - *pBoxLow;
  *pBoxBuffer    = *pBoxSize * pParams->settings[BUFFER_BOX_MULTIPLE];
  
  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[primaryShift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"loadIndicators() InstanceID = %d, BarTime = %s, ATR = %lf, BoxLow = %lf, BoxHigh = %lf, BoxSize = %lf, BoxBuffer = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, *pATR, *pBoxLow, *pBoxHigh, *pBoxSize, *pBoxBuffer);

  returnCode = setUIValues(pParams, *pATR, *pBoxLow, *pBoxHigh, *pBoxSize, *pBoxBuffer);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("loadIndicators()", returnCode);
  }

  return SUCCESS;
}

static AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, double takeProfit)
{
  AsirikuyReturnCode returnCode = SUCCESS;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(hasEntrySignal((int)pParams->results[0].tradingSignals) || hasUpdateSignal((int)pParams->results[0].tradingSignals))
  {
    return SUCCESS;
  }
  
  returnCode = checkInternalTP(pParams, PRIMARY_RATES, 0, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }

  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }

  return SUCCESS;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, double stopLoss, double takeProfit, double atr, double boxSize, double boxHigh, double boxLow, double boxBuffer, time_t marketAnalysisTime)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  int shift0Index, shift1Index;
  time_t currentTime;
  time_t virtualOrderEntryTime;
  struct tm timeInfo;
  char       timeString1[MAX_TIME_STRING_SIZE] = "";
  char       timeString2[MAX_TIME_STRING_SIZE] = "";

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pParams = NULL");
    return NULL_POINTER;
  }
  
  virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);

  shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
  shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  currentTime = pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index];
  safe_gmtime(&timeInfo, currentTime);
  
  safe_timeString(timeString1, marketAnalysisTime);
  //safe_timeString(timeString2, virtualOrderEntryTime);

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Last entry signal time = %s, Last breakout box ending time = %s, Time Difference (hours) = %lf", timeString2, timeString1, difftime(virtualOrderEntryTime,marketAnalysisTime)/3600); 
  
  if ((virtualOrderEntryTime > marketAnalysisTime) && (virtualOrderEntryTime != -1) && fabs(difftime(virtualOrderEntryTime,marketAnalysisTime)) < 3600*24*5)
  {
	 pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Entry signal already triggered for breakout. No trade allowed.");
	 return SUCCESS;
  }

  if(  (boxSize < (pParams->accountInfo.minimumStop + pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0]))
    || (boxSize > (atr * pParams->settings[MAX_BOX_SIZE_ATR]))
    || (boxSize < (atr * pParams->settings[MIN_BOX_SIZE_ATR])))
  {
    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"handleTradeEntries() Invalid box size. InstanceID = %d, boxSize = %lf, minBoxSize = %lf, maxBoxSize = %lf, spread + minStop = %lf", 
      (int)pParams->settings[STRATEGY_INSTANCE_ID], boxSize, atr * pParams->settings[MIN_BOX_SIZE_ATR], atr * pParams->settings[MAX_BOX_SIZE_ATR], pParams->accountInfo.minimumStop + pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0]);
    return SUCCESS;
  }

  if(pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] > (boxHigh + boxBuffer))
  {
    time_t orderUpdateTime = setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], currentTime, (BOOL)pParams->settings[IS_BACKTESTING]);

    if( pParams->settings[ENTRY_LOGIC] == TRADE_BOX_BREAKOUTS )
    {
      return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
    }
    else if(pParams->settings[ENTRY_LOGIC] == FADE_BOX_BREAKOUTS)
    {
      return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
    }
  }
  else if(pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] < (boxLow - boxBuffer))
  {
    time_t orderUpdateTime = setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], currentTime, (BOOL)pParams->settings[IS_BACKTESTING]);

    if(pParams->settings[ENTRY_LOGIC] == TRADE_BOX_BREAKOUTS)
    {
      return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
    }
    else if(pParams->settings[ENTRY_LOGIC] == FADE_BOX_BREAKOUTS)
    {
      return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
    }
  }

  return SUCCESS;
}

AsirikuyReturnCode runAtipaq(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  double     atr, boxHigh, boxLow, boxSize, boxBuffer, stopLoss, takeProfit;
  int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
  time_t     marketAnalysisTime;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runAtipaq() failed. pParams = NULL");
    return NULL_POINTER;
  }

  returnCode = validateAtipaqParameters(pParams);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runAtipaq()", returnCode);
  }

  returnCode = loadIndicators(pParams, &atr, &boxSize, &boxHigh, &boxLow, &boxBuffer, &marketAnalysisTime);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runAtipaq()", returnCode);
  }

  stopLoss   = boxSize + boxBuffer + (boxSize * pParams->settings[MOVE_SL_BOX_MULTIPLE]);
  takeProfit = boxSize * pParams->settings[PROFIT_BOX_MULTIPLE];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  returnCode = handleTradeEntries(pParams, stopLoss, takeProfit, atr, boxSize, boxHigh, boxLow, boxBuffer, marketAnalysisTime);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runAtipaq()", returnCode);
  }

  returnCode = handleTradeExits(pParams, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runAtipaq()", returnCode);
  }

	return SUCCESS;
}
