/**
 * @file
 * @brief     The Watukushay - For Everyone trading system.
 * @details   WatukushayFE is a portfolio of 3 strategies trading on a 1 hour timeframe.
 * @details   The first strategy trades using Bollinger Bands.
 * @details   The second strategy trades using the Commodity Channel Index (CCI).
 * @details   The third strategy trades using the Relative Strength Index (RSI).
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported WatukushayFE to the F4 framework)
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
#include "EasyTradeCWrapper.hpp"

#define USE_INTERNAL_SL_RSI FALSE
#define USE_INTERNAL_TP_RSI TRUE

#define USE_INTERNAL_SL_BB FALSE
#define USE_INTERNAL_TP_BB FALSE

#define USE_INTERNAL_SL_CCI FALSE
#define USE_INTERNAL_TP_CCI TRUE

typedef enum additionalSettings_t
{
	RSI_PERIOD               = ADDITIONAL_PARAM_1,
	RSI_ENTRY_LEVEL          = ADDITIONAL_PARAM_2,
  RSI_EXIT_LEVEL           = ADDITIONAL_PARAM_3,
  RSI_USE_MONDAY_FILTER		 = ADDITIONAL_PARAM_4,

  BB_PERIOD                = ADDITIONAL_PARAM_1,
  BB_DEVIATION             = ADDITIONAL_PARAM_2,
  BB_BUFFER_ATR_MULTIPLIER = ADDITIONAL_PARAM_3,

  CCI_PERIOD               = ADDITIONAL_PARAM_1,
  CCI_CROSS_LEVEL          = ADDITIONAL_PARAM_2
} AdditionalSettings;

AsirikuyReturnCode runWatukushayFE_RSI(StrategyParams* pParams)
{
  const int  CURRENT_RSI_SHIFT = 1, PREVIOUS_RSI_SHIFT = 2;
  AsirikuyReturnCode returnCode = SUCCESS;
  TA_RetCode retCode;
  struct tm  timeInfo;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atr, currentRsi, previousRsi, stopLoss, takeProfit;
  int        tradingSignals = 0, outBegIdx, outNBElement, shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  int        shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3, dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;
   
  // custom ui variable definitions
  char *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
                                           "ATR ",
										   "Current RSI ", 
										   "Previous RSI ",
										   " ",
										   " ",
										   " ",
										   " ",
										   " ",
										   " ",
										   " "
											};

  double userInterfaceValues[TOTAL_UI_VALUES];

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runWatukushayFE_RSI() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  retCode  = TA_ATR(dailyShift1Index, dailyShift1Index, pParams->ratesBuffers->rates[DAILY_RATES].high, pParams->ratesBuffers->rates[DAILY_RATES].low, pParams->ratesBuffers->rates[DAILY_RATES].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  retCode = TA_RSI(shift1Index, shift1Index, pParams->ratesBuffers->rates[PRIMARY_RATES].close, (int)pParams->settings[RSI_PERIOD], &outBegIdx, &outNBElement, &currentRsi);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_RSI()", retCode);
  }

  retCode = TA_RSI(shift2Index, shift2Index, pParams->ratesBuffers->rates[PRIMARY_RATES].close, (int)pParams->settings[RSI_PERIOD], &outBegIdx, &outNBElement, &previousRsi);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_RSI()", retCode);
  }

  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"WatukushayFE_RSI InstanceID = %d, BarTime = %s, ATR = %lf, CurrentRSI = %lf, PreviousRSI = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, currentRsi, previousRsi);
  
   // set ui values
	userInterfaceValues[0] = atr ;
	userInterfaceValues[1] = currentRsi ;
	userInterfaceValues[2] = previousRsi ;
	userInterfaceValues[3] = 0 ;
	userInterfaceValues[4] = 0 ;
  userInterfaceValues[5] = 0 ;
	userInterfaceValues[6] = 0 ;
	userInterfaceValues[7] = 0 ;
	userInterfaceValues[8] = 0 ;
	userInterfaceValues[9] = 0 ;

  // call ui file saving
	saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, 10,  (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

  stopLoss   = atr * pParams->settings[SL_ATR_MULTIPLIER];
  takeProfit = atr * pParams->settings[TP_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL_RSI, USE_INTERNAL_TP_RSI);
  }

  safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  if((timeInfo.tm_wday != 1) || (pParams->settings[RSI_USE_MONDAY_FILTER] != 1))
  {
    if((previousRsi > (100 - pParams->settings[RSI_ENTRY_LEVEL])) && (currentRsi < 100 - pParams->settings[RSI_ENTRY_LEVEL]))
    {
      return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL_RSI, USE_INTERNAL_TP_RSI);
    }
    else if((previousRsi < (pParams->settings[RSI_ENTRY_LEVEL])) && (currentRsi  > pParams->settings[RSI_ENTRY_LEVEL]))
    {
      return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL_RSI, USE_INTERNAL_TP_RSI);
    }
  }
  
  returnCode = checkInternalTP(pParams, PRIMARY_RATES, 0, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runWatukushayFE_RSI()", returnCode);
  }
  
  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL_RSI, USE_INTERNAL_TP_RSI);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runWatukushayFE_RSI()", returnCode);
  }

  if((totalOpenOrders(pParams, BUY) > 0) && (previousRsi < 100 - pParams->settings[RSI_EXIT_LEVEL]) && (currentRsi  > 100 - pParams->settings[RSI_EXIT_LEVEL]))
  {
    returnCode = closeLongTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runWatukushayFE_RSI()", returnCode);
    }
  }

  if((totalOpenOrders(pParams, SELL) > 0) && (previousRsi > pParams->settings[RSI_EXIT_LEVEL]) && (currentRsi  < pParams->settings[RSI_EXIT_LEVEL]))
  {
    returnCode = closeShortTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runWatukushayFE_RSI()", returnCode);
    }
  }

	return SUCCESS;
}

AsirikuyReturnCode runWatukushayFE_BB(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
	TA_RetCode retCode;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atr, upperBand, middleBand, lowerBand, stopLoss, takeProfit;
  int        tradingSignals = 0, outBegIdx, outNBElement, shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  int        dailyShift0Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 1;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runWatukushayFE_BB() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  atr = iAtrSafeShiftZero((int)parameter(ATR_AVERAGING_PERIOD));

  retCode = TA_BBANDS(shift1Index, shift1Index, pParams->ratesBuffers->rates[PRIMARY_RATES].close, (int)pParams->settings[BB_PERIOD], pParams->settings[BB_DEVIATION], pParams->settings[BB_DEVIATION], TA_MAType_SMA, &outBegIdx, &outNBElement, &upperBand, &middleBand, &lowerBand);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_BBANDS()", retCode);
  }

  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"WatukushayFE_BB InstanceID = %d, BarTime = %s, ATR = %lf, UpperBollingerBand = %lf, LowerBollingerBand = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, upperBand, lowerBand);

  stopLoss   = atr * pParams->settings[SL_ATR_MULTIPLIER];
  takeProfit = atr * pParams->settings[TP_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL_BB, USE_INTERNAL_TP_BB);
  }

  if(pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] > (upperBand + pParams->settings[BB_BUFFER_ATR_MULTIPLIER] * atr) && (hour() > 10) && (hour() < 21))
  {
    return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL_BB, USE_INTERNAL_TP_BB);
  }
  else if(pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] < (lowerBand - pParams->settings[BB_BUFFER_ATR_MULTIPLIER] * atr) && (hour() > 10) && (hour() < 21))
  {
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL_BB, USE_INTERNAL_TP_BB);
  }

  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL_BB, USE_INTERNAL_TP_BB);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }
  
	return SUCCESS;
}

AsirikuyReturnCode runWatukushayFE_CCI(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
	TA_RetCode retCode;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atr, cci, stopLoss, takeProfit;
  int        tradingSignals = 0, outBegIdx, outNBElement;
  int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  int        dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runWatukushayFE_CCI() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  retCode  = TA_ATR(dailyShift1Index, dailyShift1Index, pParams->ratesBuffers->rates[DAILY_RATES].high, pParams->ratesBuffers->rates[DAILY_RATES].low, pParams->ratesBuffers->rates[DAILY_RATES].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  retCode = TA_CCI(shift1Index, shift1Index, pParams->ratesBuffers->rates[PRIMARY_RATES].high, pParams->ratesBuffers->rates[PRIMARY_RATES].low, pParams->ratesBuffers->rates[PRIMARY_RATES].close, (int)pParams->settings[CCI_PERIOD], &outBegIdx, &outNBElement, &cci);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_CCI()", retCode);
  }

  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"WatukushayFE_CCI InstanceID = %d, BarTime = %s, ATR = %lf, CCI = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, cci);

  stopLoss   = atr * pParams->settings[SL_ATR_MULTIPLIER];
  takeProfit = atr * pParams->settings[TP_ATR_MULTIPLIER];

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL_CCI, USE_INTERNAL_TP_CCI);
  }

  if(cci > pParams->settings[CCI_CROSS_LEVEL])
  {
    return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL_CCI, USE_INTERNAL_TP_CCI);
  }
  else if(cci < (-1 * pParams->settings[CCI_CROSS_LEVEL]))
  {
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL_CCI, USE_INTERNAL_TP_CCI);
  }
  
  returnCode = checkInternalTP(pParams, PRIMARY_RATES, 0, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runWatukushayFE_CCI()", returnCode);
  }

  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL_CCI, USE_INTERNAL_TP_CCI);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }
  
	return SUCCESS;
}
