/**
 * @file
 * @brief     The Sapaq trading system.
 * @details   Sapaq uses a 1 hour timeframe and looks for counter-trending moves at the end of the asian session.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported Sapaq to the F4 framework)
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
#include "CriticalSection.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP TRUE

typedef enum additionalSettings_t
{
	SESSION_MOVE_MIN          = ADDITIONAL_PARAM_1,
	SESSION_MOVE_MAX          = ADDITIONAL_PARAM_2,
	SESSION_PERIOD            = ADDITIONAL_PARAM_3,
	MARKET_ANALYSIS_HOUR      = ADDITIONAL_PARAM_4, 
	USE_EURUSD_MONTHLY_FILTER = ADDITIONAL_PARAM_5,
	USE_USDJPY_MONTHLY_FILTER = ADDITIONAL_PARAM_6,
	USE_GBPUSD_MONTHLY_FILTER = ADDITIONAL_PARAM_7

} AdditionalSettings;

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, struct tm* pTimeInfo, double openSession, double asianSessionRun, double stopLoss, double takeProfit, double atr)
{
  int shift0Index, shift1Index, shift24Index, twoPipsFactor;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pParams = NULL");
    return NULL_POINTER;
  }
  
  if(pTimeInfo == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pTimeInfo = NULL");
    return NULL_POINTER;
  }
  
	shift0Index  = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
  shift1Index  = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  shift24Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 25;

  if (Digits() == 3 || Digits()  == 5)
	  twoPipsFactor = 20;

  if (Digits() == 2 || Digits()  == 4)
	  twoPipsFactor = 2;


  if(  (pTimeInfo->tm_hour !=  pParams->settings[MARKET_ANALYSIS_HOUR])
    || (asianSessionRun >= (atr * pParams->settings[SESSION_MOVE_MAX]))
    || (asianSessionRun <= (atr * pParams->settings[SESSION_MOVE_MIN]))
    || ((pTimeInfo->tm_mon > 9 ) && (pParams->settings[USE_EURUSD_MONTHLY_FILTER] == 1))
	  || (((pTimeInfo->tm_mon == 5 || pTimeInfo->tm_mon == 7))  && (pParams->settings[USE_USDJPY_MONTHLY_FILTER] == 1))
	  || (((pTimeInfo->tm_mon == 0 || pTimeInfo->tm_mon == 1 || pTimeInfo->tm_mon == 8 || pTimeInfo->tm_mon == 9)) && (pParams->settings[USE_GBPUSD_MONTHLY_FILTER] == 1)))
  {
    return SUCCESS;
  }
	
  if(((openSession - pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index]) > (twoPipsFactor * pParams->ratesBuffers->rates[PRIMARY_RATES].info.point))
    && (pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] < pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift24Index]))
	{
    return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
	else if(((pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] - openSession) > (twoPipsFactor * pParams->ratesBuffers->rates[PRIMARY_RATES].info.point))
    && (pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index] > pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift24Index]))
	{
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
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

AsirikuyReturnCode runSapaq(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  TA_RetCode retCode;
	struct tm  timeInfo;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
	double     atr, openSession, asianSessionRun, stopLoss, takeProfit;
	int        tradingSignals = 0, outBegIdx, outNBElement;
  int        shift0Index, shift1Index, shift24Index, dailyShift1Index;

   // custom ui variable definitions
  char *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
                                           "ATR ",
										   "Corrected Time", 
										   "Analysis Time ",
										   "Last session range ",
										   "Last close",
										   "Close 24 bars ago ",
										   " ",
										   " ",
										   " ",
										   " "
											};

  double userInterfaceValues[TOTAL_UI_VALUES];

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runSapaq() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
	shift0Index      = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
  shift1Index      = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  shift24Index     = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 25;
  dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;

 retCode = TA_ATR(dailyShift1Index, dailyShift1Index, pParams->ratesBuffers->rates[DAILY_RATES].high, pParams->ratesBuffers->rates[DAILY_RATES].low, pParams->ratesBuffers->rates[DAILY_RATES].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
	if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_ATR()", retCode);
    return TA_LIB_ERROR;
  }

  openSession = pParams->ratesBuffers->rates[PRIMARY_RATES].open[(int)(shift0Index - pParams->settings[SESSION_PERIOD])];

  retCode = TA_ATR(shift1Index, shift1Index, pParams->ratesBuffers->rates[PRIMARY_RATES].high, pParams->ratesBuffers->rates[PRIMARY_RATES].low, pParams->ratesBuffers->rates[PRIMARY_RATES].close, (int)pParams->settings[SESSION_PERIOD], &outBegIdx, &outNBElement, &asianSessionRun);
	if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_ATR()", retCode);
    return TA_LIB_ERROR;
  }

  safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Sapaq InstanceID = %d, BarTime = %s, ATR = %lf, OpenSession = %lf, AsianSessionRun = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, openSession, asianSessionRun);
  
   // set ui values
	userInterfaceValues[0] = atr ;
	userInterfaceValues[1] = timeInfo.tm_hour ;
	userInterfaceValues[2] = pParams->settings[MARKET_ANALYSIS_HOUR] ;
	userInterfaceValues[3] = asianSessionRun ;
	userInterfaceValues[4] = pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index]  ;
  userInterfaceValues[5] = pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift24Index] ;
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
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  returnCode = handleTradeEntries(pParams, &timeInfo, openSession, asianSessionRun, stopLoss, takeProfit, atr);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runSapaq()", returnCode);
  }
  
  returnCode = handleTradeExits(pParams, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runAtipaq()", returnCode);
  }

  return SUCCESS;
}
