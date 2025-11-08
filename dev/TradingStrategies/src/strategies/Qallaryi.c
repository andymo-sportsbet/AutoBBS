/**
 * @file
 * @brief     The Qallaryi trading system.
 * @details   Qallaryi uses a 1 hour timeframe and trades based on moving average crosses.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported Qallaryi to the F4 framework)
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
#include "StrategyUserInterface.h"
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
  CLOSING_MA_ENABLED = ADDITIONAL_PARAM_1,
  CLOSE_ON_REVERSE   = ADDITIONAL_PARAM_2,
  FAST_MA_PERIOD     = ADDITIONAL_PARAM_3,
  SLOW_MA_PERIOD     = ADDITIONAL_PARAM_4,
  CLOSE_MA_PERIOD    = ADDITIONAL_PARAM_5,
  MA_METHOD          = ADDITIONAL_PARAM_6,
  MA_APPLIED_PRICE   = ADDITIONAL_PARAM_7,
  USE_TIME_FILTER    = ADDITIONAL_PARAM_8,
  HOUR_0             = ADDITIONAL_PARAM_9,
  HOUR_1             = ADDITIONAL_PARAM_10,
  HOUR_2             = ADDITIONAL_PARAM_11,
  HOUR_3             = ADDITIONAL_PARAM_12,
  HOUR_4             = ADDITIONAL_PARAM_13,
  HOUR_5             = ADDITIONAL_PARAM_14,
  HOUR_6             = ADDITIONAL_PARAM_15,
  HOUR_7             = ADDITIONAL_PARAM_16,
  HOUR_8             = ADDITIONAL_PARAM_17,
  HOUR_9             = ADDITIONAL_PARAM_18,
  HOUR_10            = ADDITIONAL_PARAM_19,
  HOUR_11            = ADDITIONAL_PARAM_20,
  HOUR_12            = ADDITIONAL_PARAM_21,
  HOUR_13            = ADDITIONAL_PARAM_22,
  HOUR_14            = ADDITIONAL_PARAM_23,
  HOUR_15            = ADDITIONAL_PARAM_24,
  HOUR_16             = ADDITIONAL_PARAM_25,
  HOUR_17             = ADDITIONAL_PARAM_26,
  HOUR_18             = ADDITIONAL_PARAM_27,
  HOUR_19             = ADDITIONAL_PARAM_28,
  HOUR_20             = ADDITIONAL_PARAM_29,
  HOUR_21             = ADDITIONAL_PARAM_30,
  HOUR_22             = ADDITIONAL_PARAM_31,
  HOUR_23             = ADDITIONAL_PARAM_32,
} AdditionalSettings;

typedef enum ratesIndexes_t
{
  PRIMARY_RATES = 0,
  DAILY_RATES   = 1
} RatesIndexes;

typedef struct indicators_t
{
  double atr;
  double fastMaPrev;
  double fastMaNow;
  double slowMaPrev;
  double slowMaNow;
  double closeMaPrev;
  double closeMaNow;
} Indicators;

static BOOL fastSlowUpCross(double fastMaPrev, double slowMaPrev, double fastMaNow, double slowMaNow)
{
  return((fastMaPrev < slowMaPrev) && (fastMaNow > slowMaNow));
}

static BOOL fastSlowDownCross(double fastMaPrev, double slowMaPrev, double fastMaNow, double slowMaNow)
{
  return((fastMaPrev > slowMaPrev) && (fastMaNow < slowMaNow));
}

static BOOL fastCloseDownCross(double fastMaPrev, double closeMaPrev, double fastMaNow, double closeMaNow)
{
  return((fastMaPrev > closeMaPrev) && (fastMaNow < closeMaNow));
}

static BOOL fastCloseUpCross(double fastMaPrev, double closeMaPrev, double fastMaNow, double closeMaNow)
{
  return((fastMaPrev < closeMaPrev) && (fastMaNow > closeMaNow));
}

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators, struct tm* pTimeInfo)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  double userInterfaceValues[TOTAL_UI_VALUES];
  char   *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
    "ATR",
    "Fast MA Previous",
    "Slow MA Previous",
    "Fast MA Current",
    "Slow MA Current",
    "Corrected Hour",
    "",
    "",
    "",
    ""};

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"setUIValues() failed. pParams = NULL");
    return NULL_POINTER;
  }

  userInterfaceValues[0] = pIndicators->atr;
	userInterfaceValues[1] = pIndicators->fastMaPrev;
	userInterfaceValues[2] = pIndicators->slowMaPrev;
	userInterfaceValues[3] = pIndicators->fastMaNow;
	userInterfaceValues[4] = pIndicators->slowMaNow;
  userInterfaceValues[5] = pTimeInfo->tm_hour;
	userInterfaceValues[6] = 0;
	userInterfaceValues[7] = 0;
	userInterfaceValues[8] = 0;
	userInterfaceValues[9] = 0;

	return saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, TOTAL_UI_VALUES, (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  TA_RetCode retCode;
  struct tm  timeInfo;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double*    price;
  int        outBegIdx, outNBElement, shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
  int        dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;

  retCode = TA_ATR(dailyShift1Index, dailyShift1Index, pParams->ratesBuffers->rates[DAILY_RATES].high, pParams->ratesBuffers->rates[DAILY_RATES].low, pParams->ratesBuffers->rates[DAILY_RATES].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &pIndicators->atr);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  switch((AppliedPrice)pParams->settings[MA_APPLIED_PRICE])
  {
  case PRICE_CLOSE:
    {
      price = pParams->ratesBuffers->rates[PRIMARY_RATES].close;
      break;
    }
  case PRICE_OPEN:
    {
      price = pParams->ratesBuffers->rates[PRIMARY_RATES].open;
      break;
    }
  case PRICE_HIGH:
    {
      price = pParams->ratesBuffers->rates[PRIMARY_RATES].high;
      break;
    }
  case PRICE_LOW:
    {
      price = pParams->ratesBuffers->rates[PRIMARY_RATES].low;
      break;
    }
  case PRICE_MEDIAN:
    {
      int i;
      price = (double*)malloc(pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize * sizeof(double));
      for(i = 0; i < pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize; i++)
      {
        price[i] = (pParams->ratesBuffers->rates[PRIMARY_RATES].high[i] + pParams->ratesBuffers->rates[PRIMARY_RATES].low[i]) / 2;
      }
      break;
    }
  case PRICE_TYPICAL:
    {
      int i;
      price = (double*)malloc(pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize * sizeof(double));
      for(i = 0; i < pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize; i++)
      {
        price[i] = (pParams->ratesBuffers->rates[PRIMARY_RATES].high[i] + pParams->ratesBuffers->rates[PRIMARY_RATES].low[i] + pParams->ratesBuffers->rates[PRIMARY_RATES].close[i]) / 3;
      }
      break;
    }
  case PRICE_WEIGHTED:
    {
      int i;
      price = (double*)malloc(pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize * sizeof(double));
      for(i = 0; i < pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize; i++)
      {
        price[i] = (pParams->ratesBuffers->rates[PRIMARY_RATES].high[i] + pParams->ratesBuffers->rates[PRIMARY_RATES].low[i] + (2 * pParams->ratesBuffers->rates[PRIMARY_RATES].close[i])) / 4;
      }
      break;
    }
  default:
    {
      pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"loadIndicators() failed. Invalid applied price. pParams->settings[MA_APPLIED_PRICE] = %f", pParams->settings[MA_APPLIED_PRICE]);
      return TA_LIB_ERROR;
    }
  }

  retCode  = TA_MA(shift2Index, shift2Index, price, (int)pParams->settings[FAST_MA_PERIOD], (TA_MAType)pParams->settings[MA_METHOD], &outBegIdx, &outNBElement, &pIndicators->fastMaPrev);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_MA()", retCode);
    return TA_LIB_ERROR;
  }

  retCode  = TA_MA(shift1Index, shift1Index, price, (int)pParams->settings[FAST_MA_PERIOD], (TA_MAType)pParams->settings[MA_METHOD], &outBegIdx, &outNBElement, &pIndicators->fastMaNow);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_MA()", retCode);
    return TA_LIB_ERROR;
  }

  retCode  = TA_MA(shift2Index, shift2Index, price, (int)pParams->settings[SLOW_MA_PERIOD], (TA_MAType)pParams->settings[MA_METHOD], &outBegIdx, &outNBElement, &pIndicators->slowMaPrev);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_MA()", retCode);
    return TA_LIB_ERROR;
  }

  retCode  = TA_MA(shift1Index, shift1Index, price, (int)pParams->settings[SLOW_MA_PERIOD], (TA_MAType)pParams->settings[MA_METHOD], &outBegIdx, &outNBElement, &pIndicators->slowMaNow);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_MA()", retCode);
    return TA_LIB_ERROR;
  }

  retCode  = TA_MA(shift2Index, shift2Index, price, (int)pParams->settings[CLOSE_MA_PERIOD], (TA_MAType)pParams->settings[MA_METHOD], &outBegIdx, &outNBElement, &pIndicators->closeMaPrev);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_MA()", retCode);
    return TA_LIB_ERROR;
  }

  retCode  = TA_MA(shift1Index, shift1Index, price, (int)pParams->settings[CLOSE_MA_PERIOD], (TA_MAType)pParams->settings[MA_METHOD], &outBegIdx, &outNBElement, &pIndicators->closeMaNow);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_MA()", retCode);
    return TA_LIB_ERROR;
  }

  /* Free any allocated memory */
  switch((AppliedPrice)pParams->settings[MA_APPLIED_PRICE])
  {
  case PRICE_MEDIAN:
  case PRICE_TYPICAL:
  case PRICE_WEIGHTED:
    {
      free(price);
      break;
    }
  default:
    {
      break;
    }
  }

  safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Qallaryi InstanceID = %d, BarTime = %s, ATR = %lf, fastMaPrev = %lf, fastMaNow = %lf, slowMaPrev = %lf, slowMaNow = %lf, closeMaPrev = %lf, closeMaNow = %lf"
    , (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr, pIndicators->fastMaPrev, pIndicators->fastMaNow, pIndicators->slowMaPrev, pIndicators->slowMaNow, pIndicators->closeMaPrev, pIndicators->closeMaNow);
  
  returnCode = setUIValues(pParams, pIndicators, &timeInfo);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("loadIndicators()", returnCode);
  }

  return SUCCESS;
}

static AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, Indicators* pIndicators, double takeProfit)
{
  AsirikuyReturnCode returnCode = SUCCESS;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pIndicators == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pIndicators = NULL");
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

  if(totalOpenOrders(pParams, BUY) > 0)
  {
    if((pParams->settings[CLOSING_MA_ENABLED] && fastCloseDownCross(pIndicators->fastMaPrev, pIndicators->closeMaPrev, pIndicators->fastMaNow, pIndicators->closeMaNow))
      || (pParams->settings[CLOSE_ON_REVERSE] && fastSlowDownCross(pIndicators->fastMaPrev, pIndicators->slowMaPrev, pIndicators->fastMaNow, pIndicators->slowMaNow)))
    {
      returnCode = closeLongTrade(pParams, 0);
      if(returnCode != SUCCESS)
      {
        return logAsirikuyError("handleTradeExits()", returnCode);
      }
    }
  }

  if(totalOpenOrders(pParams, SELL) > 0)
  {
    if((pParams->settings[CLOSING_MA_ENABLED] && fastCloseUpCross(pIndicators->fastMaPrev, pIndicators->closeMaPrev, pIndicators->fastMaNow, pIndicators->closeMaNow))
      || (pParams->settings[CLOSE_ON_REVERSE] && fastSlowUpCross(pIndicators->fastMaPrev, pIndicators->slowMaPrev, pIndicators->fastMaNow, pIndicators->slowMaNow)))
    {
      returnCode = closeShortTrade(pParams, 0);
      {
        return logAsirikuyError("handleTradeExits()", returnCode);
      }
    }
  }
  
  return SUCCESS;
}

BOOL tradeCurrentHour( int hour){

	if (hour == 0 && HOUR_0 == 1) return TRUE;
	if (hour == 1 && HOUR_1 == 1) return TRUE;
	if (hour == 2 && HOUR_2 == 1) return TRUE;
	if (hour == 3 && HOUR_3 == 1) return TRUE;
	if (hour == 4 && HOUR_4 == 1) return TRUE;
	if (hour == 5 && HOUR_5 == 1) return TRUE;
	if (hour == 6 && HOUR_6 == 1) return TRUE;
	if (hour == 7 && HOUR_7 == 1) return TRUE;
	if (hour == 8 && HOUR_8 == 1) return TRUE;
	if (hour == 9 && HOUR_9 == 1) return TRUE;
	if (hour == 10 && HOUR_10 == 1) return TRUE;
	if (hour == 11 && HOUR_11 == 1) return TRUE;
	if (hour == 12 && HOUR_12 == 1) return TRUE;
	if (hour == 13 && HOUR_13 == 1) return TRUE;
	if (hour == 14 && HOUR_14 == 1) return TRUE;
	if (hour == 15 && HOUR_15 == 1) return TRUE;
	if (hour == 16 && HOUR_16 == 1) return TRUE;
	if (hour == 17 && HOUR_17 == 1) return TRUE;
	if (hour == 18 && HOUR_18 == 1) return TRUE;
	if (hour == 19 && HOUR_19 == 1) return TRUE;
	if (hour == 20 && HOUR_20 == 1) return TRUE;
	if (hour == 21 && HOUR_21 == 1) return TRUE;
	if (hour == 22 && HOUR_22 == 1) return TRUE;
	if (hour == 23 && HOUR_23 == 1) return TRUE;

	return FALSE;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, double stopLoss, double takeProfit)
{
  struct tm  timeInfo;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pIndicators == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pIndicators = NULL");
    return NULL_POINTER;
  }

  safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[PRIMARY_RATES].time[pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1]);
  if(pParams->settings[USE_TIME_FILTER] && !tradeCurrentHour(timeInfo.tm_hour))
  {
    /* Don't trade this hour */
    return SUCCESS;
  }

  if(fastSlowUpCross(pIndicators->fastMaPrev, pIndicators->slowMaPrev, pIndicators->fastMaNow, pIndicators->slowMaNow))
  {
    return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  else if(fastSlowDownCross(pIndicators->fastMaPrev, pIndicators->slowMaPrev, pIndicators->fastMaNow, pIndicators->slowMaNow))
  {
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  return SUCCESS;
}

AsirikuyReturnCode runQallaryi(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  Indicators indicators;
  double     stopLoss, takeProfit;
  int        tradingSignals = 0, shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runQallaryi() failed. pParams = NULL");
    return NULL_POINTER;
  }

  returnCode = loadIndicators(pParams, &indicators);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runQallaryi()", returnCode);
  }

  stopLoss   = indicators.atr * pParams->settings[SL_ATR_MULTIPLIER];
  takeProfit = indicators.atr * pParams->settings[TP_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  
  returnCode = handleTradeEntries(pParams, &indicators, stopLoss, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runQallaryi()", returnCode);
  }
  
  returnCode = handleTradeExits(pParams, &indicators, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runQallaryi()", returnCode);
  }
  
  return SUCCESS;
}