#include "Precompiled.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include <ta_libc.h>
#include <ta_func.h>
#include "AsirikuyImageProcessorCWrapper.hpp"

typedef enum additionalSettings_t
{
  MA_PERIOD         = ADDITIONAL_PARAM_1, 
} AdditionalSettings;

void freeRenko(StrategyParams* pParams){
   free(pParams->ratesBuffers->rates[2].time);
   free(pParams->ratesBuffers->rates[2].open);
   free(pParams->ratesBuffers->rates[2].high);
   free(pParams->ratesBuffers->rates[2].low);
   free(pParams->ratesBuffers->rates[2].close);
   free(pParams->ratesBuffers->rates[2].volume);
}

AsirikuyReturnCode runRenkoTest(StrategyParams* pParams)
{
   double atr, stopLoss, takeProfit;
   int periodfast=10;
   int periodslow=100;
   
   TA_RetCode retCode;
  int        outBegIdx, outNBElement, shift = 1;
  double	  Aroonslow, Aroonfast1, Aroonfast2;
  int shift0Index = pParams->ratesBuffers->rates[2].info.arraySize - 1 ;
  int shift1Index = pParams->ratesBuffers->rates[2].info.arraySize - 2 ;
   
   atr = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
   
  addNewRenkoRates(PRIMARY_RATES,2,0.001);

  //chartAllInputData(2, 600, 350);

  retCode = TA_AROONOSC(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[2].high, pParams->ratesBuffers->rates[2].low, periodslow, &outBegIdx, &outNBElement, &Aroonslow);
      if(retCode != TA_SUCCESS)
      {
        logTALibError("TA_AROON()", retCode);
        return TA_LIB_ERROR;
      }

  retCode = TA_AROONOSC(shift0Index-shift, shift0Index-shift, pParams->ratesBuffers->rates[2].high, pParams->ratesBuffers->rates[2].low, periodfast, &outBegIdx, &outNBElement, &Aroonfast1);
      if(retCode != TA_SUCCESS)
      {
        logTALibError("TA_AROON()", retCode);
        return TA_LIB_ERROR;
      }

   retCode = TA_AROONOSC(shift1Index-shift, shift1Index-shift, pParams->ratesBuffers->rates[2].high, pParams->ratesBuffers->rates[2].low, periodfast, &outBegIdx, &outNBElement, &Aroonfast2);
      if(retCode != TA_SUCCESS)
      {
        logTALibError("TA_AROON()", retCode);
        return TA_LIB_ERROR;
      }


   //plotAroonOsc(pParams, 100, 100, 600, 350);
   //plotAroonOsc(pParams, 10, 100, 600, 350);		
   addValueToUI("ATR", atr);

   stopLoss   = atr*parameter(SL_ATR_MULTIPLIER);
   takeProfit = atr*parameter(TP_ATR_MULTIPLIER);

   pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"Slow = %lf, Fast2 = %lf, Fast1 = %lf", Aroonslow, Aroonfast2, Aroonfast1);

   if ((Aroonslow>=0) && (Aroonfast2<=-50) && (Aroonfast1>-50)){
	   pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"OPEN SHORT");
	   freeRenko(pParams);
		return openOrUpdateLongEasy(takeProfit, stopLoss,1);
   }

   if ((Aroonslow<=0) && (Aroonfast2>=50) && (Aroonfast1<50)){
	   pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"OPEN LONG");
	   freeRenko(pParams);
	   return openOrUpdateShortEasy(takeProfit, stopLoss,1);
   }

   if (buyOrdersCount()!=0 && Aroonslow<0)
   {
	   closeAllLongs();
   }

   if (sellOrdersCount()!=0 && Aroonslow>0)
   {
	   closeAllShorts();
   }

   freeRenko(pParams);
	return SUCCESS;
}