/**
 * @file
 * @brief     The Comitl trading system.
 * @details   Comitl is a portfolio of 3 strategies trading on a daily timeframe.
 * @details   The first strategy trades using price action.
 * @details   The second strategy trades using Bollinger Bands
 * @details   The third strategy trades using Keltner Channels
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported Comitl to the F4 framework)
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

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

typedef enum additionalSettings_t
{
  PA_LOOK_BACK_DAYS    = ADDITIONAL_PARAM_1,
  PA_EXIT_DAYS         = ADDITIONAL_PARAM_2,

  BB_PERIOD            = ADDITIONAL_PARAM_1,
  BB_DEVIATION         = ADDITIONAL_PARAM_2,
  BB_INITIAL_MA_PERIOD = ADDITIONAL_PARAM_3,

  KC_MA_PERIOD         = ADDITIONAL_PARAM_1,
  KC_CLOSE_MA_PERIOD   = ADDITIONAL_PARAM_2,
  KC_ATR_PERIOD        = ADDITIONAL_PARAM_3,
  KC_DISTANCE          = ADDITIONAL_PARAM_4
} AdditionalSettings;

AsirikuyReturnCode runComitl_PA(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  TA_RetCode retCode;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atr, lowCriteria, highCriteria, stopLoss;
  int        i, result = SUCCESS, tradingSignals = 0, outBegIdx, outNBElement;
  int        shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[0].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[0].info.arraySize - 3;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runComitl_PA() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  retCode = TA_ATR(shift1Index, shift1Index, pParams->ratesBuffers->rates[0].high, pParams->ratesBuffers->rates[0].low, pParams->ratesBuffers->rates[0].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  lowCriteria = pParams->ratesBuffers->rates[0].close[shift2Index] ;
  highCriteria = pParams->ratesBuffers->rates[0].close[shift2Index] ;

  for(i = 3; i < pParams->settings[PA_LOOK_BACK_DAYS]; i++) 
  {
    if (pParams->ratesBuffers->rates[0].close[shift0Index - i] > highCriteria)
    {
      highCriteria = pParams->ratesBuffers->rates[0].close[shift0Index - i];
    }

    if (pParams->ratesBuffers->rates[0].close[shift0Index - i] < lowCriteria)
    {
      lowCriteria = pParams->ratesBuffers->rates[0].close[shift0Index - i];
    }
  }
  
  safe_timeString(timeString, pParams->ratesBuffers->rates[0].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Comitl_PA InstanceID = %d, BarTime = %s, ATR = %lf, LowCriteria = %lf, HighCriteria = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, lowCriteria, highCriteria);

  stopLoss = atr * pParams->settings[SL_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, 0, 0, stopLoss, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  
  if((pParams->ratesBuffers->rates[0].close[shift1Index] > highCriteria) && (pParams->orderInfo[0].openTime < pParams->ratesBuffers->rates[0].time[shift0Index]))
  {
    return openOrUpdateLongTrade(pParams, 0, 0, stopLoss, 0, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  else if((pParams->ratesBuffers->rates[0].close[shift1Index] < lowCriteria) && (pParams->orderInfo[0].openTime < pParams->ratesBuffers->rates[0].time[shift0Index]))
  {
    return openOrUpdateShortTrade(pParams, 0, 0, stopLoss, 0, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  if((pParams->orderInfo[0].openTime != 0) 
    && ((pParams->ratesBuffers->rates[0].time[shift0Index] - pParams->orderInfo[0].openTime)  >  (SECONDS_PER_DAY * pParams->settings[PA_EXIT_DAYS])))
  {
    returnCode = closeLongTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runComitl_PA()", returnCode);
    }

    returnCode = closeShortTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runComitl_PA()", returnCode);
    }
  }
  
	return SUCCESS;
}

AsirikuyReturnCode runComitl_BB(StrategyParams* pParams)
{
  const int  MINIMUM_MA_PERIOD = 10;
  AsirikuyReturnCode returnCode = SUCCESS;
  TA_RetCode retCode;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atr, upperBand, middleBand, lowerBand, averageClose, stopLoss;
  int        tradingSignals = 0, outBegIdx, outNBElement, maPeriod;
  int        shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[0].info.arraySize - 2;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runComitl_BB() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  retCode = TA_ATR(shift1Index, shift1Index, pParams->ratesBuffers->rates[0].high, pParams->ratesBuffers->rates[0].low, pParams->ratesBuffers->rates[0].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  retCode = TA_BBANDS(shift1Index, shift1Index, pParams->ratesBuffers->rates[0].close, (int)pParams->settings[BB_PERIOD], pParams->settings[BB_DEVIATION], pParams->settings[BB_DEVIATION], TA_MAType_SMA, &outBegIdx, &outNBElement, &upperBand, &middleBand, &lowerBand);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_BBANDS()", retCode);
  }

  maPeriod = (int)pParams->settings[BB_INITIAL_MA_PERIOD] - (int)((pParams->ratesBuffers->rates[0].time[shift0Index] - pParams->orderInfo[0].openTime) / SECONDS_PER_DAY);
  if(maPeriod < MINIMUM_MA_PERIOD)
  {
    maPeriod = MINIMUM_MA_PERIOD;
  }

  retCode = TA_SMA(shift1Index, shift1Index, pParams->ratesBuffers->rates[0].close, maPeriod, &outBegIdx, &outNBElement, &averageClose);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_SMA()", retCode);
  }
  
  safe_timeString(timeString, pParams->ratesBuffers->rates[0].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Comitl_BB InstanceID = %d, BarTime = %s, ATR = %lf, AverageClose = %lf, UpperBB = %lf, MiddleBB = %lf, LowerBB = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, averageClose, upperBand, middleBand, lowerBand);
  
  stopLoss = atr * pParams->settings[SL_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, 0, 0, stopLoss, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  if((pParams->ratesBuffers->rates[0].close[shift1Index] > upperBand) && (pParams->ratesBuffers->rates[0].close[shift1Index] > averageClose))
  {
    return openOrUpdateLongTrade(pParams, 0, 0, stopLoss, 0, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  else if((pParams->ratesBuffers->rates[0].close[shift1Index] < lowerBand) && (pParams->ratesBuffers->rates[0].close[shift1Index] < averageClose))
  {
    return openOrUpdateShortTrade(pParams, 0, 0, stopLoss, 0, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  
  if((totalOpenOrders(pParams, BUY) > 0) && (pParams->ratesBuffers->rates[0].close[shift1Index] < averageClose))
  {
    returnCode = closeLongTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runComitl_BB()", returnCode);
    }
  }

  if((totalOpenOrders(pParams, SELL) > 0) && (pParams->ratesBuffers->rates[0].close[shift1Index] > averageClose))
  {
    returnCode = closeShortTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runComitl_BB()", returnCode);
    }
  }

	return SUCCESS;
}

AsirikuyReturnCode runComitl_KC(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  TA_RetCode retCode;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atr, upperKC, lowerKC, middleKC, averageClose, stopLoss;
  int        tradingSignals = 0, outBegIdx, outNBElement;
  int        shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[0].info.arraySize - 2;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runComitl_KC() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  retCode  = TA_ATR(shift1Index, shift1Index, pParams->ratesBuffers->rates[0].high, pParams->ratesBuffers->rates[0].low, pParams->ratesBuffers->rates[0].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  retCode = TA_SMA(shift1Index, shift1Index, pParams->ratesBuffers->rates[0].close, (int)pParams->settings[KC_CLOSE_MA_PERIOD], &outBegIdx, &outNBElement, &averageClose);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_SMA()", retCode);
  }

  returnCode = calculateKeltnerChannels(pParams->ratesBuffers->rates[0].high, pParams->ratesBuffers->rates[0].low, pParams->ratesBuffers->rates[0].close, pParams->ratesBuffers->rates[0].info.arraySize, (int)pParams->settings[KC_ATR_PERIOD], (int)pParams->settings[KC_MA_PERIOD], pParams->settings[KC_DISTANCE], pParams->settings[KC_DISTANCE], STANDARD_INDICATOR_SHIFT, &upperKC, &middleKC, &lowerKC);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("keltnerChannels()", returnCode);
  }
  
  safe_timeString(timeString, pParams->ratesBuffers->rates[0].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Comitl_KC InstanceID = %d, BarTime = %s, ATR = %lf, AverageClose = %lf, UpperKC = %lf, MiddleKC = %lf, LowerKC = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, averageClose, upperKC, middleKC, lowerKC);
  
  stopLoss = atr * pParams->settings[SL_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, 0, 0, stopLoss, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  if((pParams->ratesBuffers->rates[0].close[shift1Index] > upperKC) && (pParams->ratesBuffers->rates[0].close[shift1Index] > averageClose))
  {
    return openOrUpdateLongTrade(pParams, 0, 0, stopLoss, 0,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  else if((pParams->ratesBuffers->rates[0].close[shift1Index] < lowerKC) && (pParams->ratesBuffers->rates[0].close[shift1Index] < averageClose))
  {
    return openOrUpdateShortTrade(pParams, 0, 0, stopLoss, 0,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  
  if((totalOpenOrders(pParams, BUY) > 0) && (pParams->ratesBuffers->rates[0].close[shift1Index] < averageClose))
  {
    returnCode = closeLongTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runComitl_KC()", returnCode);
    }
  }

  if((totalOpenOrders(pParams, SELL) > 0) && (pParams->ratesBuffers->rates[0].close[shift1Index] > averageClose))
  {
    returnCode = closeShortTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runComitl_KC()", returnCode);
    }
  }

	return SUCCESS;
}
