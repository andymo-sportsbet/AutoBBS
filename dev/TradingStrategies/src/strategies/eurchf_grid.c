/**
 * @file
 * @brief     The EURCHF Grid trading system.
 * @details   This is a grid system designed to trade with the assumption that the SNB floor at 1.2 will hold. Do NOT trade on any other pair.
 *
 * @author    Daniel Fernandez (initial F4 implementation)
 * @author    Morgan Doel (F4 framework functionality used)
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
#include <time.h>
#include <math.h>

#include "AsirikuyStrategies.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "EasyTradeCWrapper.hpp"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP TRUE

typedef enum additionalSettings_t
{
	MAX_POSITIONS     = ADDITIONAL_PARAM_1,
	TOTAL_RISK	      = ADDITIONAL_PARAM_2,
	INDEPENDENT_TP	  = ADDITIONAL_PARAM_3,
	CONDITIONAL_TIMED_EXIT = ADDITIONAL_PARAM_4
} AdditionalSettings;


AsirikuyReturnCode runEURCHF_grid(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  double     stopLoss, takeProfit;
  int        i;
  double gridSpace = 0, maxEntry, minEntry, newEntry = 0;


  // custom ui variable definitions
  char *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
                                           "TradeCount ",
										   "GridSpace", 
										   "NextEntry",
										   "PipsToTP",
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
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runEURCHF_grid() failed. pParams = NULL");
    return NULL_POINTER;
  }

  stopLoss   = pParams->bidAsk.ask[0]-1.1950;
  takeProfit = 0.0015;

  if (parameter(INDEPENDENT_TP) != 0)
	  takeProfit = parameter(INDEPENDENT_TP);

   pParams->settings[ACCOUNT_RISK_PERCENT] = parameter(TOTAL_RISK)/parameter(MAX_POSITIONS);

  maxEntry = 0;
  minEntry = 1000;

  if(totalOpenOrders(pParams, BUY) > 0)
  {

	  for(i = 0; i < pParams->settings[ORDERINFO_ARRAY_SIZE]; i++)
		{		
			
			if((!pParams->orderInfo[i].isOpen) || (pParams->orderInfo[i].instanceId != (int)pParams->settings[STRATEGY_INSTANCE_ID]))
			{
			continue;
			}

			if(pParams->orderInfo[i].openPrice > maxEntry)
				maxEntry = pParams->orderInfo[i].openPrice;

			if(pParams->orderInfo[i].openPrice < minEntry)
				minEntry = pParams->orderInfo[i].openPrice;

			if (parameter(INDEPENDENT_TP) == 0)
			takeProfit = fabs(pParams->bidAsk.ask[0]-pParams->orderInfo[i].takeProfit);

			 pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Position %d: Age Days=%lf, Profit/loss (absolute price) = %lf", i, (pParams->currentBrokerTime-pParams->orderInfo[i].openTime)/(1440*60), pParams->bidAsk.ask[0]-pParams->orderInfo[i].openPrice);

			// conditional time exit, if profitable close a trade if X days have passed since open.
			if(	(parameter(CONDITIONAL_TIMED_EXIT) != 0) &&
				((int)((pParams->currentBrokerTime-pParams->orderInfo[i].openTime))/(1440*60) > parameter(CONDITIONAL_TIMED_EXIT)) &&
				(pParams->bidAsk.ask[0] > pParams->orderInfo[i].openPrice)){

			closeLongEasy(pParams->orderInfo[i].ticket);

			}
		} 

	  gridSpace = (maxEntry-1.2)/parameter(MAX_POSITIONS);
	  newEntry = minEntry - gridSpace;

	 
  }

  // set ui values
	userInterfaceValues[0] = totalOpenOrders(pParams, BUY) ;
	userInterfaceValues[1] = gridSpace;
	userInterfaceValues[2] = newEntry ;
	userInterfaceValues[3] = takeProfit ;
	userInterfaceValues[4] = 0 ;
    userInterfaceValues[5] = 0 ;
	userInterfaceValues[6] = 0 ;
	userInterfaceValues[7] = 0 ;
	userInterfaceValues[8] = 0 ;
	userInterfaceValues[9] = 0 ;

	// call ui file saving
	saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, 10,  (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);



  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  if(totalOpenOrders(pParams, BUY) > 0 && pParams->bidAsk.ask[0] < newEntry)
	  return openSingleLongEasy(takeProfit, stopLoss, 0,1);

   if(totalOpenOrders(pParams, BUY) == 0)
	  return openSingleLongEasy(takeProfit, stopLoss, 0,1);


	return SUCCESS;
}
