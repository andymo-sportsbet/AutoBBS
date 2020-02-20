/**
 * @file
 * @brief     The Teyacanani trading system.
 * @details   Teyacanani uses a 1 hour timeframe and trades based on candle size.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported Teyacanani to the F4 framework)
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
#include <stdlib.h>

#include "AsirikuyStrategies.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderSignals.h"
#include "EasyTradeCWrapper.hpp"
#include "OrderManagement.h"
#include "AsirikuyTechnicalAnalysis.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

#define BAR_RANGE_BODY_RATIO 1.5

typedef enum additionalSettings_t
{
	OPEN_ATR_MULTIPLIER  = ADDITIONAL_PARAM_1,
	CLOSE_ATR_MULTIPLIER = ADDITIONAL_PARAM_2,
	TRADE_TARGET         = ADDITIONAL_PARAM_3,
	TRADE_CONFIDENCE     = ADDITIONAL_PARAM_4

} AdditionalSettings;

typedef struct indicators_t
{
  double atr;
  double open;
  double close;
  double barRange;
  double barBody;
} Indicators;

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, double atr, double barRange)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  double userInterfaceValues[TOTAL_UI_VALUES];
  char   *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
    "ATR",
    "Bar Range",
    "",
    "",
    "",
    "",
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
	userInterfaceValues[1] = barRange;
	userInterfaceValues[2] = 0;
	userInterfaceValues[3] = 0;
	userInterfaceValues[4] = 0;
  userInterfaceValues[5] = 0;
	userInterfaceValues[6] = 0;
	userInterfaceValues[7] = 0;
	userInterfaceValues[8] = 0;
	userInterfaceValues[9] = 0;

	return saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, TOTAL_UI_VALUES, (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
  AsirikuyReturnCode returnCode;
  char timeString[MAX_TIME_STRING_SIZE] = "";
  int  shift0Index      = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
  int  shift1Index      = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  int  dailyShift0Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 1;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pIndicators == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pIndicators = NULL");
    return NULL_POINTER;
  }

    //pIndicators->atr = iRangeAverage(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
    pIndicators->atr = iAtrSafeShiftZero((int)parameter(ATR_AVERAGING_PERIOD));
	
    pIndicators->open     = pParams->ratesBuffers->rates[PRIMARY_RATES].open[shift1Index];
	pIndicators->close    = pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index];
	pIndicators->barRange = fabs(pParams->ratesBuffers->rates[PRIMARY_RATES].high[shift1Index] - pParams->ratesBuffers->rates[PRIMARY_RATES].low[shift1Index]);
	pIndicators->barBody  = fabs(pIndicators->open - pIndicators->close);
	if(pIndicators->barBody < 0.00001)
  {
    /* Avoid dividing by 0 */
    pIndicators->barBody = 0.00001;
  }

  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Teyacanani indicators: InstanceID = %d, BarTime = %s, open = %lf, close = %lf, BarRange = %lf, BarBody = %lf, ATR = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->open, pIndicators->close, pIndicators->barRange, pIndicators->barBody, pIndicators->atr);
  returnCode = setUIValues(pParams, pIndicators->atr, pIndicators->barRange);
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

  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }

  if((totalOpenOrders(pParams, BUY) != 0) && (pIndicators->barRange / pIndicators->barBody < BAR_RANGE_BODY_RATIO) && (pIndicators->barBody > pIndicators->atr * pParams->settings[CLOSE_ATR_MULTIPLIER]) && (pIndicators->open > pIndicators->close))
  {
    returnCode = closeLongTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("handleTradeExits()", returnCode);
    }
  }

  if((totalOpenOrders(pParams, SELL) != 0) && (pIndicators->barRange / pIndicators->barBody < BAR_RANGE_BODY_RATIO) && (pIndicators->barBody > pIndicators->atr * pParams->settings[CLOSE_ATR_MULTIPLIER]) && (pIndicators->open < pIndicators->close))
  {
    returnCode = closeShortTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("handleTradeExits()", returnCode);
    }
  }

  return SUCCESS;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, double stopLoss, double takeProfit)
{

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

  if((pIndicators->barRange / pIndicators->barBody < BAR_RANGE_BODY_RATIO) && (pIndicators->barBody > pIndicators->atr * pParams->settings[OPEN_ATR_MULTIPLIER]) && (pIndicators->open < pIndicators->close) && (hour() < 21) && (hour() > 2) && (month() != 9))
	{
    return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
	else if((pIndicators->barRange / pIndicators->barBody < BAR_RANGE_BODY_RATIO) && (pIndicators->barBody > pIndicators->atr * pParams->settings[OPEN_ATR_MULTIPLIER]) && (pIndicators->open > pIndicators->close) && (hour() < 21) && (hour() > 2) && (month() != 9))
	{
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, 1,USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  return SUCCESS;
}

AsirikuyReturnCode runTeyacanani(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  Indicators indicators;
  double stopLoss, takeProfit;
  int shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runTeyacanani() failed. pParams = NULL");
    return NULL_POINTER;
  }

  returnCode = loadIndicators(pParams, &indicators);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runTeyacanani()", returnCode);
  }

  stopLoss   =  CalculateEllipticalStopLoss(pParams, parameter(TRADE_TARGET), (int)parameter(TIMED_EXIT_BARS), parameter(TRADE_CONFIDENCE), 1);
  takeProfit =  CalculateEllipticalTakeProfit(pParams, parameter(TRADE_TARGET), (int)parameter(TIMED_EXIT_BARS), parameter(TRADE_CONFIDENCE), 1);

  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  returnCode = handleTradeEntries(pParams, &indicators, stopLoss, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runTeyacanani()", returnCode);
  }
  
  returnCode = handleTradeExits(pParams, &indicators, takeProfit);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runTeyacanani()", returnCode);
  }

  if ( (totalOpenOrders(pParams, NONE) > 0) && (hasExitSignal((int)pParams->results[0].tradingSignals) == FALSE) )
  {
	if( totalOpenOrders(pParams, BUY) > 0) 
		{
			stopLoss   = fabs(pParams->bidAsk.ask[0]-(cOpen(getOrderAge(pParams, 0)) - CalculateEllipticalStopLoss(pParams, parameter(TRADE_TARGET), (int)parameter(TIMED_EXIT_BARS), parameter(TRADE_CONFIDENCE), getOrderAge(pParams, 0)+1)));
			takeProfit = fabs(pParams->bidAsk.ask[0]-(cOpen(getOrderAge(pParams, 0)) + CalculateEllipticalTakeProfit(pParams, parameter(TRADE_TARGET),(int)parameter(TIMED_EXIT_BARS), parameter(TRADE_CONFIDENCE), getOrderAge(pParams, 0)+1)));
			modifyTradeEasy(BUY , -1, stopLoss, takeProfit);
		}
	if(totalOpenOrders(pParams, SELL) > 0) 
		{
			stopLoss   = fabs(pParams->bidAsk.bid[0]-(cOpen(getOrderAge(pParams, 0)) + CalculateEllipticalStopLoss(pParams, parameter(TRADE_TARGET), (int)parameter(TIMED_EXIT_BARS), parameter(TRADE_CONFIDENCE), getOrderAge(pParams, 0)+1)));
			takeProfit = fabs(pParams->bidAsk.bid[0]-(cOpen(getOrderAge(pParams, 0)) - CalculateEllipticalTakeProfit(pParams, parameter(TRADE_TARGET),(int)parameter(TIMED_EXIT_BARS), parameter(TRADE_CONFIDENCE), getOrderAge(pParams, 0)+1)));
			modifyTradeEasy(SELL, -1, stopLoss, takeProfit);
		}	
  }
  
  return SUCCESS;
}
