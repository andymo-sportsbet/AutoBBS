/**
 * @file
 * @brief     The God's Gift ATR trading system.
 * @details   God's Gift ATR uses a 1 hour timeframe and enters trades on trend retracements.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported God's Gift ATR to the F4 framework)
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
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "StrategyUserInterface.h"
#include "EasyTradeCWrapper.hpp"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

typedef enum additionalSettings_t
{
  BB_PERIOD                  = ADDITIONAL_PARAM_1,
  BB_DEVIATION               = ADDITIONAL_PARAM_2,
  KC_PERIOD                  = ADDITIONAL_PARAM_3,
  KC_ATR_DISTANCE            = ADDITIONAL_PARAM_4,
  SHORT_TERM_STOCH_PERIOD    = ADDITIONAL_PARAM_5,
  MEDIUM_TERM_STOCH_PERIOD   = ADDITIONAL_PARAM_6,
  LONG_TERM_STOCH_PERIOD     = ADDITIONAL_PARAM_7,
  SHORT_STOCH_EXIT_SYMMETRY  = ADDITIONAL_PARAM_8,
  MED_STOCH_EXIT_SYMMETRY    = ADDITIONAL_PARAM_9,
  LONG_STOCH_EXIT_SYMMETRY   = ADDITIONAL_PARAM_10,
  SHORT_STOCH_ENTRY_SYMMETRY = ADDITIONAL_PARAM_11,
  MED_STOCH_ENTRY_SYMMETRY   = ADDITIONAL_PARAM_12,
  LONG_STOCH_ENTRY_SYMMETRY  = ADDITIONAL_PARAM_13,
  EURUSD_TIME_FILTER		 = ADDITIONAL_PARAM_14
} AdditionalSettings;

static BOOL upTrendSlowDown(
  double* pSettings, 
  double  currentPrice,  
  double  bbUpper, 
  double  bbLower, 
  double  shortStochShift1, 
  double  shortStochShift2, 
  double  medStochShift1, 
  double  medStochShift2, 
  double  medStochShift3, 
  double  longStochShift1, 
  double  longStochShift2)
{

  return(  (( (longStochShift1 < pSettings[LONG_STOCH_EXIT_SYMMETRY]) && (medStochShift2 < pSettings[MED_STOCH_EXIT_SYMMETRY]))
    || (shortStochShift1 > pSettings[SHORT_STOCH_EXIT_SYMMETRY])
    || ((currentPrice > bbUpper) && (longStochShift1 < pSettings[LONG_STOCH_EXIT_SYMMETRY]))));
}

static BOOL downTrendSlowDown(
  double* pSettings, 
  double  currentPrice, 
  double  bbUpper, 
  double  bbLower, 
  double  shortStochShift1, 
  double  shortStochShift2, 
  double  medStochShift1, 
  double  medStochShift2, 
  double  medStochShift3, 
  double  longStochShift1, 
  double  longStochShift2)
{
  return( (((longStochShift1 > 100-pSettings[LONG_STOCH_EXIT_SYMMETRY]) && (medStochShift2 > 100-pSettings[MED_STOCH_EXIT_SYMMETRY]))
    || (shortStochShift1 < (100 - pSettings[SHORT_STOCH_EXIT_SYMMETRY]))
    || ((currentPrice < bbLower)  && (longStochShift1 > 100-pSettings[LONG_STOCH_EXIT_SYMMETRY]))));
}

static BOOL upTrendRetracement(
  double* pSettings, 
  double  currentPrice, 
  double  highShift1, 
  double  lowShift1,  
  double  bbUpper, 
  double  bbLower,  
  double  upperKC1, 
  double  shortStochShift1, 
  double  shortStochShift2, 
  double  medStochShift1, 
  double  medStochShift2, 
  double  medStochShift3, 
  double  longStochShift1, 
  double  longStochShift2)
{
  return((longStochShift1 > pSettings[LONG_STOCH_ENTRY_SYMMETRY])
    &&   (medStochShift1  > pSettings[MED_STOCH_ENTRY_SYMMETRY])
    &&   (currentPrice < bbUpper)
    &&   (currentPrice < upperKC1)
    &&   (shortStochShift1 < pSettings[SHORT_STOCH_ENTRY_SYMMETRY])
    &&   (highShift1 < bbUpper));
}

static BOOL downTrendRetracement(
  double* pSettings, 
  double  currentPrice, 
  double  highShift1, 
  double  lowShift1, 
  double  bbUpper, 
  double  bbLower, 
  double  lowerKC1, 
  double  shortStochShift1, 
  double  shortStochShift2, 
  double  medStochShift1, 
  double  medStochShift2, 
  double  medStochShift3, 
  double  longStochShift1, 
  double  longStochShift2)
{
  return((longStochShift1 < (100 - pSettings[LONG_STOCH_ENTRY_SYMMETRY]))
    &&   (medStochShift1  < (100 - pSettings[MED_STOCH_ENTRY_SYMMETRY]))
    &&   (currentPrice > bbLower)
    &&   (currentPrice > lowerKC1)
    &&   (shortStochShift1 > (100 - pSettings[SHORT_STOCH_ENTRY_SYMMETRY]))
    &&   (lowShift1 > bbLower));
}

static AsirikuyReturnCode loadIndicators(
  StrategyParams* pParams, 
  double*         pAtr, 
  double*         pShortStochShift1, 
  double*         pShortStochShift2, 
  double*         pMedStochShift1, 
  double*         pMedStochShift2, 
  double*         pMedStochShift3, 
  double*         pLongStochShift1, 
  double*         pLongStochShift2, 
  double*         pBBUpper, 
  double*         pBBLower, 
  double*         pUpperKC1, 
  double*         pMiddleKC1, 
  double*         pLowerKC1)
{
  const int    STOCH_SLOW_K = 3, STOCH_SLOW_D = 1;
  const int    KC_SHIFT = 1;

  int        outBegIdx, outNBElement, standardShift0Index, standardShift1Index, standardShift2Index, standardShift3Index, dailyShift0Index;
  double     notUsed;
  Rates      *standardRates, *dailyRates;
  TA_RetCode retCode;
  AsirikuyReturnCode returnCode = SUCCESS;

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"loadIndicators() failed. pParams = NULL");
    return NULL_POINTER;
  }

  standardRates = &pParams->ratesBuffers->rates[PRIMARY_RATES];
  dailyRates    = &pParams->ratesBuffers->rates[DAILY_RATES];
  standardShift0Index = standardRates->info.arraySize - 1;
  standardShift1Index = standardRates->info.arraySize - 2;
  standardShift2Index = standardRates->info.arraySize - 3;
  standardShift3Index = standardRates->info.arraySize - 4;
  dailyShift0Index    = dailyRates->info.arraySize - 1;

  *pAtr = iAtrSafeShiftZero((int)parameter(ATR_AVERAGING_PERIOD));

  retCode = TA_STOCH(standardShift1Index, standardShift1Index, standardRates->high, standardRates->low, standardRates->close, (int)pParams->settings[SHORT_TERM_STOCH_PERIOD], STOCH_SLOW_K, TA_MAType_SMA, STOCH_SLOW_D, TA_MAType_SMA, &outBegIdx, &outNBElement, pShortStochShift1, &notUsed);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_STOCH(pShortStochShift1)", retCode);
  }

  retCode = TA_STOCH(standardShift2Index, standardShift2Index, standardRates->high, standardRates->low, standardRates->close, (int)pParams->settings[SHORT_TERM_STOCH_PERIOD], STOCH_SLOW_K, TA_MAType_SMA, STOCH_SLOW_D, TA_MAType_SMA, &outBegIdx, &outNBElement, pShortStochShift2, &notUsed);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_STOCH(pShortStochShift2)", retCode);
  }

  retCode = TA_STOCH(standardShift1Index, standardShift1Index, standardRates->high, standardRates->low, standardRates->close, (int)pParams->settings[MEDIUM_TERM_STOCH_PERIOD], STOCH_SLOW_K, TA_MAType_SMA, STOCH_SLOW_D, TA_MAType_SMA, &outBegIdx, &outNBElement, pMedStochShift1, &notUsed);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_STOCH(pShortStochShift1)", retCode);
  }

  retCode = TA_STOCH(standardShift2Index, standardShift2Index, standardRates->high, standardRates->low, standardRates->close, (int)pParams->settings[MEDIUM_TERM_STOCH_PERIOD], STOCH_SLOW_K, TA_MAType_SMA, STOCH_SLOW_D, TA_MAType_SMA, &outBegIdx, &outNBElement, pMedStochShift2, &notUsed);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_STOCH(pShortStochShift2)", retCode);
  }

  retCode = TA_STOCH(standardShift3Index, standardShift3Index, standardRates->high, standardRates->low, standardRates->close, (int)pParams->settings[MEDIUM_TERM_STOCH_PERIOD], STOCH_SLOW_K, TA_MAType_SMA, STOCH_SLOW_D, TA_MAType_SMA, &outBegIdx, &outNBElement, pMedStochShift3, &notUsed);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_STOCH(pShortStochShift3)", retCode);
  }

  retCode = TA_STOCH(standardShift1Index, standardShift1Index, standardRates->high, standardRates->low, standardRates->close, (int)pParams->settings[LONG_TERM_STOCH_PERIOD], STOCH_SLOW_K, TA_MAType_SMA, STOCH_SLOW_D, TA_MAType_SMA, &outBegIdx, &outNBElement, pLongStochShift1, &notUsed);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_STOCH(pLongStochShift1)", retCode);
  }

  retCode = TA_STOCH(standardShift2Index, standardShift2Index, standardRates->high, standardRates->low, standardRates->close, (int)pParams->settings[LONG_TERM_STOCH_PERIOD], STOCH_SLOW_K, TA_MAType_SMA, STOCH_SLOW_D, TA_MAType_SMA, &outBegIdx, &outNBElement, pLongStochShift2, &notUsed);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_STOCH(pLongStochShift2)", retCode);
  }

  retCode = TA_BBANDS(standardShift1Index, standardShift1Index, standardRates->close, (int)pParams->settings[BB_PERIOD], pParams->settings[BB_DEVIATION], pParams->settings[BB_DEVIATION], TA_MAType_SMA, &outBegIdx, &outNBElement, pBBUpper, &notUsed, pBBLower);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_BBANDS()", retCode);
  }

  returnCode = calculateKeltnerChannels(standardRates->high, standardRates->low, standardRates->close, standardRates->info.arraySize, (int)pParams->settings[KC_PERIOD], (int)pParams->settings[KC_PERIOD], pParams->settings[KC_ATR_DISTANCE],  pParams->settings[KC_ATR_DISTANCE], KC_SHIFT, pUpperKC1, pMiddleKC1, pLowerKC1);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("keltnerChannels()", returnCode);
  }

  return SUCCESS;
}

AsirikuyReturnCode runGodsGiftATR(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  char       timeString[MAX_TIME_STRING_SIZE] = "";
  double     bbUpper, bbLower, upperKC1, middleKC1, lowerKC1;
  double     atr, shortStochShift1, shortStochShift2, medStochShift1, medStochShift2, medStochShift3, longStochShift1, longStochShift2, currentPrice, stopLoss, takeProfit;
  int        tradingSignals = 0, shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;

    // custom ui variable definitions
  char *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
                                           "ATR ",
										   "Short STO ",
										   "Middle STO ",
										   "Long STO ",
										   "Keltner Upper ",
										   "Keltner Middle ",
										   "Keltner Lower ",
										   "BB Upper ",
										   "BB Lower ",
										   " ",
											};

  double userInterfaceValues[TOTAL_UI_VALUES];

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runGodsGiftATR() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  returnCode = loadIndicators(pParams, &atr, &shortStochShift1, &shortStochShift2, &medStochShift1, 
    &medStochShift2, &medStochShift3, &longStochShift1, &longStochShift2, &bbUpper, &bbLower, &upperKC1, &middleKC1, &lowerKC1);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("runGodsGiftATR()", returnCode);
  }

  currentPrice = pParams->ratesBuffers->rates[PRIMARY_RATES].open[shift0Index];
  
  safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"GodsGiftATR InstanceID = %d, BarTime = %s, ATR = %lf, ShortStochShift1 = %lf, ShortStochShift2 = %lf, MedStochShift1 = %lf, MedStochShift2 = %lf, MedStochShift3 = %lf, LongStochShift1 = %lf, LongStochShift2 = %lf, upperBB = %lf, lowerBB = %lf, UpperKC1 = %lf, MiddleKC1 = %lf, LowerKC1 = %lf"
    , (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, shortStochShift1, shortStochShift2, medStochShift1, medStochShift2, medStochShift3, longStochShift1, longStochShift2, bbUpper, bbLower, upperKC1, middleKC1, lowerKC1);

   // set ui values
	userInterfaceValues[0] = atr ;
	userInterfaceValues[1] = shortStochShift1 ;
	userInterfaceValues[2] = medStochShift1 ;
  userInterfaceValues[3] = longStochShift1;
	userInterfaceValues[4] = upperKC1 ;
	userInterfaceValues[5] = middleKC1 ;
	userInterfaceValues[6] = lowerKC1 ;
	userInterfaceValues[7] = bbUpper ;
	userInterfaceValues[8] = bbLower ;
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

  if(upTrendRetracement(pParams->settings, pParams->ratesBuffers->rates[PRIMARY_RATES].open[shift0Index], pParams->ratesBuffers->rates[PRIMARY_RATES].high[shift1Index], pParams->ratesBuffers->rates[PRIMARY_RATES].low[shift1Index], 
     bbUpper, bbLower, upperKC1, shortStochShift1, shortStochShift2, medStochShift1, medStochShift2, medStochShift3, longStochShift1, longStochShift2))
  {
    if ((month() != 0 && month() != 1) || (parameter(EURUSD_TIME_FILTER) == 0))
    return openOrUpdateLongTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  else if(downTrendRetracement(pParams->settings, pParams->ratesBuffers->rates[PRIMARY_RATES].open[shift0Index], pParams->ratesBuffers->rates[PRIMARY_RATES].high[shift1Index], pParams->ratesBuffers->rates[PRIMARY_RATES].low[shift1Index], 
    bbUpper, bbLower, lowerKC1, shortStochShift1, shortStochShift2, medStochShift1, medStochShift2, medStochShift3, longStochShift1, longStochShift2))
  {
	if ((month() != 0 && month() != 1) || (parameter(EURUSD_TIME_FILTER) == 0))
    return openOrUpdateShortTrade(pParams, PRIMARY_RATES, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  returnCode = checkTimedExit(pParams, PRIMARY_RATES, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  if(returnCode != SUCCESS)
  {
    return logAsirikuyError("handleTradeExits()", returnCode);
  }

  if((totalOpenOrders(pParams, BUY) > 0) 
    && upTrendSlowDown(pParams->settings, pParams->ratesBuffers->rates[PRIMARY_RATES].open[shift0Index], bbUpper, 
      bbLower, shortStochShift1, shortStochShift2, medStochShift1, medStochShift2, medStochShift3, longStochShift1, longStochShift2))
  {
    returnCode = closeLongTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runGodsGiftATR()", returnCode);
    }
  }

  if((totalOpenOrders(pParams, SELL) > 0) 
    && downTrendSlowDown(pParams->settings, pParams->ratesBuffers->rates[PRIMARY_RATES].open[shift0Index], bbUpper, 
      bbLower, shortStochShift1, shortStochShift2, medStochShift1, medStochShift2, medStochShift3, longStochShift1, longStochShift2))
  {
    returnCode = closeShortTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runGodsGiftATR()", returnCode);
    }
  }
  
  return SUCCESS;
}

