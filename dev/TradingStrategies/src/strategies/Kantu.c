/**
 * @file
 * @brief     Price Action based trading system generated with Kanut.
 * @details   This is a price action based strategy generated using the Kanut system generator.
 * 
 * @author    Daniel Fernandez (Original idea, initial F4 implementation)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Template F4 framework implementation)
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

#include <math.h>
#include <stdlib.h>
#include <ta_libc.h>

#include "AsirikuyStrategies.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "KantuStrategies.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "EasyTradeCWrapper.hpp"
#include "CriticalSection.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE


typedef enum additionalSettings_t
{
  SELECTED_STRATEGY_ID			 = ADDITIONAL_PARAM_1,
  VOLATILITY_CALCULATION_MODE    = ADDITIONAL_PARAM_2,
  TL_ATR_MULTIPLIER              = ADDITIONAL_PARAM_3,
  USE_AFMTL                      = ADDITIONAL_PARAM_4,
  SELECT_KANTU_SYSTEM_FILE       = ADDITIONAL_PARAM_5,
  DSL_BREAKEVEN_BARS             = ADDITIONAL_PARAM_6,
  DSL_TYPE                       = ADDITIONAL_PARAM_7,
  DSL_EXIT_TYPE                  = ADDITIONAL_PARAM_8
} AdditionalSettings;

typedef enum dslTypes_t
{
  DSL_NONE = 0,
  DSL_LINEAR = 1,
  DSL_LOG = 2,
  DSL_PARABOLIC = 3,
  DSL_SQUARE = 4,
  DSL_SQUARE_WITH_SKIP = 5,
  DSL_X4 = 6,
  DSL_X025 = 7
} dslTypes;

typedef enum volatilityCalculationModes_t
{
  MODE_KANTU	              = 0,
  MODE_DAILY_ATR_SHIFT_ZERO   = 1,
  MODE_DAILY_ATR_NORMAL	      = 2,
  MODE_DAILY_RANGE_SHIFT_ZERO = 3,
  MODE_DAILY_RANGE_NORMAL     = 4
} volatilityCalculationModes;

AsirikuyReturnCode runKantu(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     atrKantu, atr=0.0, currentTakeProfit, currentStopLoss, stopLoss, takeProfit, trailingStop, sumTrueRange, high, low, previousClose;
  int        i, shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
  int        dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;
  int		 orderAge, exit_Type;
  double spread;
  double newSL;
  double kantuMinStop;
  // custom ui variable definitions
  char *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
                                           "ATR ",
										   " ", 
										   " ",
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
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runKantu() failed. pParams = NULL");
    return NULL_POINTER;
  }

  // run kantu in critical section to avoid problems in MT4 execution
  enterCriticalSection();

  if ( parameter(SELECT_KANTU_SYSTEM_FILE) != 0)
	  getKantuSignal(pParams, -1, (int) parameter(SELECT_KANTU_SYSTEM_FILE), atr);

  /* Load indicators */
    sumTrueRange = 0;

	if (pParams->settings[TIMEFRAME] == 1440 )
	{
		for (i = 0; i < (int)pParams->settings[ATR_AVERAGING_PERIOD]; i++)
		{
			high = pParams->ratesBuffers->rates[PRIMARY_RATES].high[shift1Index-i];
			low  = pParams->ratesBuffers->rates[PRIMARY_RATES].low[shift1Index-i];
			previousClose = pParams->ratesBuffers->rates[PRIMARY_RATES].close[shift1Index-1-i]; 
			sumTrueRange += max( high-low, max(fabs(high-previousClose),fabs(low-previousClose)));
		}

		atrKantu = sumTrueRange/(int)pParams->settings[ATR_AVERAGING_PERIOD];
	} else {

		atrKantu = iAtrWholeDaysSimple(PRIMARY_RATES, (int)pParams->settings[ATR_AVERAGING_PERIOD]);
	}

	switch ( (int)parameter(VOLATILITY_CALCULATION_MODE))
	{
	case MODE_KANTU:
		atr = atrKantu;
		break;
	case MODE_DAILY_ATR_SHIFT_ZERO:
		atr = iAtrSafeShiftZero((int)parameter(ATR_AVERAGING_PERIOD)) ;
		break;
	case MODE_DAILY_ATR_NORMAL:
		atr = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1) ;
		break;
	case MODE_DAILY_RANGE_SHIFT_ZERO:
		atr = iRangeSafeShiftZero((int)parameter(ATR_AVERAGING_PERIOD)) ;
		break;
	case MODE_DAILY_RANGE_NORMAL:
		atr = iRangeAverage(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1) ;
		break;
	default:
		atr = atrKantu;
		break;
	}

  // set ui values
	userInterfaceValues[0] = atr ;
	userInterfaceValues[1] = 0 ;
	userInterfaceValues[2] = 0 ;
	userInterfaceValues[3] = 0 ;
	userInterfaceValues[4] = 0 ;
    userInterfaceValues[5] = 0 ;
	userInterfaceValues[6] = 0 ;
	userInterfaceValues[7] = 0 ;
	userInterfaceValues[8] = 0 ;
	userInterfaceValues[9] = 0 ;

 // call ui file saving
	saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, 10,  (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  
  stopLoss     = atr * pParams->settings[SL_ATR_MULTIPLIER];
  takeProfit   = atr * pParams->settings[TP_ATR_MULTIPLIER];
  trailingStop = atr * pParams->settings[TL_ATR_MULTIPLIER];

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Kantu System InstanceID = %d, BarTime = %s, ATR = %lf,SL_ATR_MULTIPLIER=%lf,stopLoss=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr,pParams->settings[SL_ATR_MULTIPLIER],stopLoss);

  // set minimum stop used for order modifications (trailing SL cannot be closer than this value to current price)
  if (strstr(pParams->tradeSymbol, "JPY") != NULL){
	kantuMinStop = 0.15;
  } else {
	kantuMinStop = 0.0015;
  }

  currentStopLoss = pParams->orderInfo[0].stopLoss ;
  currentTakeProfit = pParams->orderInfo[0].takeProfit ;

  // if orders were not set properly (SL or TP = 0) then set stops so
  // that the front-end can properly set these values.
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
	if( stopLoss < kantuMinStop )
		setStops(pParams, PRIMARY_RATES, 0, kantuMinStop, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
	else
		setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
  } 

  if (isLongEntrySignal(pParams, (int)pParams->settings[SELECTED_STRATEGY_ID], shift1Index, atr))
  {
	leaveCriticalSection();

	//Only update, should be not changed
	if(totalOpenOrders(pParams, BUY) > 0)
		stopLoss = fabs(pParams->bidAsk.bid[0] - currentStopLoss);
	
	//pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Kantu System InstanceID = %d, stopLoss=%lf, kantuMinStop =%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],stopLoss,kantuMinStop);

	if( stopLoss < kantuMinStop  )
		stopLoss = kantuMinStop;

	return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
	
  }
   
  if (isShortEntrySignal(pParams, (int)pParams->settings[SELECTED_STRATEGY_ID], shift1Index, atr))
  {
    leaveCriticalSection();
	//Only update, should be not changed
	if(totalOpenOrders(pParams, SELL) > 0)
		stopLoss = fabs(pParams->bidAsk.ask[0] - currentStopLoss);

	//pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Kantu System InstanceID = %d, stopLoss=%lf, kantuMinStop =%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],stopLoss,kantuMinStop);

	if( stopLoss < kantuMinStop )
		stopLoss = kantuMinStop;
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  // if we have no trades open do not execute trailing/exit conditionals (no need)
  if(totalOrdersCount() <= 0){
	  leaveCriticalSection();
	  return SUCCESS;
  }
    
  orderAge = getOrderAge(pParams, 0);

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Kantu System InstanceID = %d, CurrentStoploss = %lf, orderAge = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], currentStopLoss, (double)orderAge);

  if (trailingStop != 0){

	if ((cOpen(orderAge)-pParams->bidAsk.ask[0] > trailingStop) && (totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, trailingStop, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, trailingStop, 0);
	}

	if ((pParams->bidAsk.bid[0]-cOpen(orderAge) > trailingStop) && (totalOpenOrders(pParams, BUY) > 0) ){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, trailingStop, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, trailingStop, 0);
	}

  }

  if (parameter(USE_AFMTL) == TRUE){

	if ((pParams->bidAsk.ask[0]+stopLoss < pParams->orderInfo[0].stopLoss) &&
		(totalOpenOrders(pParams, SELL) > 0)){

		if(currentTakeProfit != 0)
		modifyTradeEasy(SELL , -1, stopLoss, fabs(currentTakeProfit-pParams->bidAsk.ask[0])); else   
		modifyTradeEasy(SELL , -1, stopLoss, 0);
	}

	if ((pParams->bidAsk.bid[0]-stopLoss > pParams->orderInfo[0].stopLoss) &&
		(totalOpenOrders(pParams, BUY) > 0) ){

		if(currentTakeProfit != 0)
		modifyTradeEasy(BUY , -1, stopLoss, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); else 
	    modifyTradeEasy(BUY , -1, stopLoss, 0);
	}

  }

  spread = fabs(pParams->bidAsk.ask[0]-pParams->bidAsk.bid[0]);
  
  exit_Type = (int) parameter(DSL_EXIT_TYPE);
  
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Testing11 : InstanceID = %d, BE = %d,orderAge = %lf,stoploss = %lf, exit_Type = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID],(int)parameter(DSL_BREAKEVEN_BARS),  (double)orderAge, stopLoss,exit_Type);

  // TODO: Use the way to follow the stop loss closer when it is in the longer term trend only in profitale area.
  /* 1. M50 2days
     2. M20 1 day
     3. Previous 2 days high/low
     4. Previous 1 day high/low
     5. 2/3 Daily ATR
     6. Orginal stop loss value. for example 1 1H ATR: ATMTL mode
  */  
  

  if (parameter(DSL_TYPE ) == DSL_LINEAR){
	  newSL = orderAge*(-stopLoss/parameter(DSL_BREAKEVEN_BARS))+stopLoss;
	  setKantuDSL(pParams,newSL,orderAge,spread,stopLoss,kantuMinStop,exit_Type);    
  }

  if (parameter(DSL_TYPE ) == DSL_LOG){
	newSL = stopLoss+(stopLoss/(-log(parameter(DSL_BREAKEVEN_BARS))))*log((double)orderAge+1);
    setKantuDSL(pParams,newSL,orderAge,spread,stopLoss,kantuMinStop,exit_Type); 
  }

  if (parameter(DSL_TYPE ) == DSL_PARABOLIC){
	newSL = stopLoss+(-stopLoss/(parameter(DSL_BREAKEVEN_BARS)*parameter(DSL_BREAKEVEN_BARS)))*((double)orderAge)*((double)orderAge);  
	setKantuDSL(pParams,newSL,orderAge,spread,stopLoss,kantuMinStop,exit_Type); 
  }

  if (parameter(DSL_TYPE ) == DSL_SQUARE){
	newSL = stopLoss+(-stopLoss/(sqrt(parameter(DSL_BREAKEVEN_BARS))))*sqrt((double)orderAge);  
	setKantuDSL(pParams,newSL,orderAge,spread,stopLoss,kantuMinStop,exit_Type); 
  }

  if (parameter(DSL_TYPE ) == DSL_SQUARE_WITH_SKIP){
	newSL = stopLoss+(-stopLoss/(sqrt(parameter(DSL_BREAKEVEN_BARS))))*sqrt((double)orderAge);
	for(i=1;i<orderAge;i++){
		if(totalOpenOrders(pParams, SELL) > 0){
			if(cOpen(i)>cClose(i)){
				newSL -= 0.5*fabs(cClose(i)-cOpen(1)) ;
			}
		}

		if(totalOpenOrders(pParams, BUY) > 0){
			if(cOpen(i)<cClose(i)){
				newSL -= 0.5*fabs(cClose(i)-cOpen(1)) ;
			}
		}
	}  
	setKantuDSL(pParams,newSL,orderAge,spread,stopLoss,kantuMinStop,exit_Type); 

  }

   if (parameter(DSL_TYPE ) == DSL_X4){
	   	newSL = stopLoss+(-stopLoss/(pow(parameter(DSL_BREAKEVEN_BARS), 4)))*pow((double)orderAge, 4);  
		setKantuDSL(pParams,newSL,orderAge,spread,stopLoss,kantuMinStop,exit_Type); 

  }

	if (parameter(DSL_TYPE ) == DSL_X025){
		newSL = stopLoss+(-stopLoss/(pow(parameter(DSL_BREAKEVEN_BARS),0.25)))*pow((double)orderAge, 0.25);  
		setKantuDSL(pParams,newSL,orderAge,spread,stopLoss,kantuMinStop,exit_Type); 
  }

  if (parameter(TIMED_EXIT_BARS) != 0){
  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);

  if(returnCode != SUCCESS)
  {
	leaveCriticalSection();
    return logAsirikuyError("runKantu()", returnCode);
  }
  

  }

  leaveCriticalSection();
  return SUCCESS ;
}
