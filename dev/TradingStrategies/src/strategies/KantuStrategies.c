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

#include <ta_libc.h>
#include <stdlib.h>

#include "AsirikuyStrategies.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "AsirikuyTechnicalAnalysis.h"
#include "KantuStrategies.h"
#include "EasyTradeCWrapper.hpp"
#include "AsirikuyMachineLearningCWrapper.hpp"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP TRUE

typedef enum mlalgoTypes_t
{
  LINEAR_REGRESSION  = 0,
  NEURAL_NETWORK	 = 1,
  K_NEAREST_NEIGHBOR = 2
} mlalgoTypes;

typedef enum sysTypes_t
{
  LONG_AND_SHORT  = 0,
  LONG_ONLY   	  = 1,
  SHORT_ONLY      = 2
} sysTypes;

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

typedef enum additionalSettingsML_t
{
    SELECT_KANTU_SYSTEM_FILE_ML = ADDITIONAL_PARAM_1,
	LEARNING_PERIOD_ML          = ADDITIONAL_PARAM_2,
	BARS_USED_ML                = ADDITIONAL_PARAM_3,
	TRADING_HOUR_ML             = ADDITIONAL_PARAM_4,
	DSL_BREAKEVEN_BARS_ML       = ADDITIONAL_PARAM_5,
	FRONTIER_ML			        = ADDITIONAL_PARAM_6,
	DSL_TYPE_ML			        = ADDITIONAL_PARAM_7,
	ML_ALGO_TYPE_ML		        = ADDITIONAL_PARAM_8,
    TL_ATR_MULTIPLIER_ML        = ADDITIONAL_PARAM_9,
    USE_AFMTL_ML                = ADDITIONAL_PARAM_10,
    TIMED_EXIT_BARS_ML          = ADDITIONAL_PARAM_11,
    DAY_FILTER_ML               = ADDITIONAL_PARAM_12
} AdditionalSettingsML;

typedef enum inputTypes_t
{
  INPUT_OPEN					 = 0,
  INPUT_HIGH					 = 1,
  INPUT_LOW		                 = 2,
  INPUT_CLOSE                    = 3,
  INPUT_BODY                     = 4,
  INPUT_RANGE                    = 5
} inputTypes;

typedef enum exitDslTypes_t
{
  EXIT_DSL_NONE = 0,
  EXIT_DSL_M50 = 1,
  EXIT_DSL_M20 = 2,
  EXIT_DSL_1DayHL = 3,
  EXIT_DSL_2DayHL = 4,
  EXIT_DSL_DailyATR = 5,
  EXIT_DSL_OrginalSL = 6  
} dslTypes;

typedef struct kantuFileParameterInfo_t
{  
  int systemFileID;
  int arraySize;
  double hourFilter;  
  double dayFilter;
  double timedExit;
  double stopLoss;
  double takeProfit;
  double AFMTL;
  double trailingStop;
  double timeFrame;
  double dslbreakeven;
  double dslType;
  double sysType;
  int *inputTypes1;
  int *inputShifts1;
  int *inputTypes2;
  int *inputShifts2;
  double *magnitudes;
} KantuFileParameterInfo;

static KantuFileParameterInfo kantuFielParams[MAX_Kantu_Systems];

void initKantuFileParams ()
{	
	int i = 0;
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Reset kantuFielParams");
	for(i=0;i<MAX_Kantu_Systems;i++)
	{
		kantuFielParams[i].systemFileID = -1;
	}
}

int countLinesInCSV(char* fileName){
	FILE                *fp;
    int                 c;             
    unsigned long       newline_count = 0;

        /* count the newline characters */

	fp = fopen(fileName, "r" );

    while ( (c=fgetc(fp)) != EOF ) {
        if ( c == '\n' )
            newline_count++;
    }

    fclose(fp);

	return newline_count;
}

double ruleConverter(int inputType, int inputShift, int signalType){
	if (signalType == BUY)
	{
		switch ( inputType ) {
		case INPUT_OPEN:
			return(cOpen(inputShift));
		 break;
		case INPUT_HIGH:
			return(high(inputShift));
		break;
		case INPUT_LOW:
			return(low(inputShift));
		break;
		case INPUT_CLOSE:
			return(cClose(inputShift));
		break;
		case INPUT_BODY:
			return(absBody(inputShift));
		break;
		case INPUT_RANGE:
			return(range(inputShift));
		break;
		default:
			return(0);
		break;
		}

	}

	if (signalType == SELL)
	{
		switch ( inputType ) {
		case INPUT_OPEN:
			return(cOpen(inputShift));
		 break;
		case INPUT_HIGH:
			return(low(inputShift));
		break;
		case INPUT_LOW:
			return(high(inputShift));
		break;
		case INPUT_CLOSE:
			return(cClose(inputShift));
		break;
		case INPUT_BODY:
			return(absBody(inputShift));
		break;
		case INPUT_RANGE:
			return(range(inputShift));
		break;
		default:
			return(0);
		break;
		}

	}
}

/*
Set new kantu dynamic stop loss when it is in a long trend.
	 0. None
     1. M50 2days
     2. M20 1 day
     3. Previous 2 days high/low
     4. Previous 1 day high/low
     5. 2/3 Daily ATR
     6. Orginal stop loss value. for example 1 1H ATR: ATMTL mode  	 
*/

void setKantuDSL( StrategyParams* pParams,double dsl,int orderAge, double spread,double stopLoss,double kantuMinStop,int exit_Type)
{
	double newSL,currentStopLoss,currentTakeProfit;	
	

	currentStopLoss = pParams->orderInfo[0].stopLoss ;
    currentTakeProfit = pParams->orderInfo[0].takeProfit ;
	// Sell order:
    // Get the closer stop loss only when it is profitalbe
    if (totalOpenOrders(pParams, SELL) > 0) {     
		  newSL = cOpen(orderAge)+ spread + dsl;
		  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Test1: Sell : InstanceID = %d, newSL = %lf, ask = %lf, current stoploss = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],newSL,pParams->bidAsk.ask[0],currentStopLoss);
		  
		  
		  if (pParams->bidAsk.ask[0]+kantuMinStop >= newSL) // Original DSL value is not appliable
		  {
			  if( exit_Type == EXIT_DSL_NONE)
				  return;

			  if (orderAge > parameter(DSL_BREAKEVEN_BARS))// Now it should be at least break event.
			  {	
				  
				  if( exit_Type == EXIT_DSL_M50)
				  {
				  		//50 Close MA
					   newSL =iMA(3, PRIMARY_RATES, 50, 1) + spread;			
				  }
			  
				  if( exit_Type == EXIT_DSL_M20)
				  {
				  		//20 Close MA
					  newSL =iMA(3, PRIMARY_RATES, 20, 1) + spread;								      
				  }

				  if( exit_Type == EXIT_DSL_1DayHL)
				  {								
					  newSL =iHigh(DAILY_RATES,1) + spread;								      
				  }
				  
				  if( exit_Type == EXIT_DSL_2DayHL)
				  {				  			
					   newSL =iHigh(DAILY_RATES,2) + spread;									      										      	
				  }
				  
				  if( exit_Type == EXIT_DSL_DailyATR)
				  {				  	
					  newSL = pParams->bidAsk.ask[0]+iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD),1) + spread;							      
				  }
				  
				  if( exit_Type == EXIT_DSL_OrginalSL)
				  {				  	
					  newSL = pParams->bidAsk.ask[0]+stopLoss + spread;							      
				  }
			  }	
			  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Test2: Sell : InstanceID = %d, newSL = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],newSL);			  
	
		  }

		  if (orderAge > parameter(DSL_BREAKEVEN_BARS) && newSL > cOpen(orderAge)- 2* spread)
					newSL = cOpen(orderAge) -  2* spread;
		  
		  if (pParams->bidAsk.ask[0]+kantuMinStop < newSL)
		  {	  
				if(currentTakeProfit != 0)
					modifyTradeEasy(SELL , -1, newSL-pParams->bidAsk.ask[0] , fabs(currentTakeProfit-pParams->bidAsk.ask[0])); 
				else  	
					modifyTradeEasy(SELL , -1, newSL-pParams->bidAsk.ask[0] , 0);
		  }

      }

    if (totalOpenOrders(pParams, BUY) > 0) 
      {
          newSL = cOpen(orderAge) - dsl;

		  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Test1: Buy : InstanceID = %d, newSL = %lf, bid=%lf, current stoploss = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],newSL,pParams->bidAsk.bid[0],currentStopLoss);

		  if (pParams->bidAsk.ask[0]+kantuMinStop < newSL) // Original DSL value is not appliable
		  {
			  if( exit_Type == EXIT_DSL_NONE)
				  return;

			  if (orderAge > parameter(DSL_BREAKEVEN_BARS))// Now it should be at least break event.
			  {	
				  
				  if( exit_Type == EXIT_DSL_M50)
				  {
				  		//50 Close MA
					   newSL =iMA(3, PRIMARY_RATES, 50, 1) - spread;			
				  }
			  
				  if( exit_Type == EXIT_DSL_M20)
				  {
				  		//20 Close MA
					  newSL =iMA(3, PRIMARY_RATES, 20, 1) - spread;								      
				  }

				  if( exit_Type == EXIT_DSL_1DayHL)
				  {								
					  newSL =iLow(DAILY_RATES,1) - spread;								      					  
				  }
				  
				  if( exit_Type == EXIT_DSL_2DayHL)
				  {				  			
					  newSL = iLow(DAILY_RATES,2) - spread;									      						  
				  }
				  
				  if( exit_Type == EXIT_DSL_DailyATR)
				  {				  	
					  newSL = pParams->bidAsk.bid[0]-iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD),1) - spread;							      
				  }				  

				  if( exit_Type == EXIT_DSL_OrginalSL)
				  {				  	
					  newSL = pParams->bidAsk.bid[0]-stopLoss - spread;							      
				  }
			  }

			 
			  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Test2: Buy : InstanceID = %d, newSL = %lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],newSL);			  
	
		  }

		if (orderAge > parameter(DSL_BREAKEVEN_BARS) && newSL < cOpen(orderAge)+ 2* spread)
					newSL = cOpen(orderAge) +  2* spread;

		if (pParams->bidAsk.bid[0]-kantuMinStop > newSL){

			  if(currentTakeProfit != 0)
				modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-newSL, fabs(currentTakeProfit-pParams->bidAsk.bid[0])); 
			  else 
				modifyTradeEasy(BUY , -1, pParams->bidAsk.bid[0]-newSL, 0);
		  }
      }      
}

int initKnatuFile(int systemFile)
{
	char fileName[250] = "";
	char tempPath[250] = "";
	char data[100] = "";
	char *ptr;
	int arraySize;
	int *inputTypes1;
	int *inputTypes2;
	int *inputShifts1;
	int *inputShifts2;
	double *magnitudes;
	int i,j;
	
	char *strtokSave;
	FILE *fp;
	
	
	for(i = 0;i< MAX_Kantu_Systems; i++)
	{
		if( kantuFielParams[i].systemFileID == systemFile )
		{
			//pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"initKnatuFile: SystemID = %d already inited", systemFile);
			return i;
		}
	}

	for(j = 0;j< MAX_Kantu_Systems; j++)
	{
		if( kantuFielParams[j].systemFileID == -1 )		
			break;		
	}
	
	requestTempFileFolderPath(tempPath);

	sprintf(fileName, "%skantu_%d.csv", tempPath, systemFile);

	arraySize = countLinesInCSV(fileName)-1;

	kantuFielParams[j].arraySize = arraySize;
	
	kantuFielParams[j].inputTypes1    =  (int*)malloc((arraySize) * sizeof(int));
	kantuFielParams[j].inputTypes2    =  (int*)malloc((arraySize) * sizeof(int));
	kantuFielParams[j].inputShifts1   =  (int*)malloc((arraySize) * sizeof(int));
	kantuFielParams[j].inputShifts2   =  (int*)malloc((arraySize) * sizeof(int));
	kantuFielParams[j].magnitudes     =  (double*)malloc((arraySize) * sizeof(double));

	inputTypes1 = kantuFielParams[j].inputTypes1;
	inputTypes2 = kantuFielParams[j].inputTypes2;
	inputShifts1 = kantuFielParams[j].inputShifts1;
	inputShifts2 = kantuFielParams[j].inputShifts2;
	magnitudes = kantuFielParams[j].magnitudes;
	
	
	i = 0;

	fp = fopen( fileName, "r" );
	if(fp == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"KantuStrategies() Failed to open strategy file.");
		free(kantuFielParams[j].inputTypes1);
		free(kantuFielParams[j].inputTypes2);
		free(kantuFielParams[j].inputShifts1);
		free(kantuFielParams[j].inputShifts2);
		free(kantuFielParams[j].magnitudes);
		free(kantuFielParams);
		return -1;
	}

    while ( fgets(data, 150, fp ) != NULL) {

		if ( i == 0 ){

		ptr = strtok_r(data, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].hourFilter));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].dayFilter));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].timedExit));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].stopLoss));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].takeProfit));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].AFMTL));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].trailingStop));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].timeFrame));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].dslbreakeven));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].dslType));
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&(kantuFielParams[j].sysType));
			
	
		} else {

		ptr = strtok_r(data, ",", &strtokSave);
		sscanf(ptr,"%d",&inputTypes1[i-1]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%d",&inputShifts1[i-1]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%d",&inputTypes2[i-1]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%d",&inputShifts2[i-1]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&magnitudes[i-1]);
			
	
		}

		i++;
	}
	kantuFielParams[j].systemFileID = systemFile;
	fclose(fp);	
	return j;
}

BOOL getKantuSignal(StrategyParams* pParams, int signalType, int systemFile, double atr){
		
	int arraySize;
	int *inputTypes1;
	int *inputTypes2;
	int *inputShifts1;
	int *inputShifts2;
	double *magnitudes;
	int i;
	double dslType = 0;
	double sysType = LONG_AND_SHORT;
	double hourFilter=-1, dayFilter=-1, AFMTL=0;
	double dslbreakeven=0, stopLoss=0, takeProfit=0, trailingStop=0, timedExit=0;
	double timeFrame;
	
	BOOL isPatternTrue;
	int index;

	

	index = initKnatuFile(systemFile);
	if(index == -1)
		return FALSE;
	/*
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,arraySize=%d", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].arraySize);	
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,trailingStop=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].trailingStop);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,AFMTL=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].AFMTL);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,timeFrame=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].timeFrame);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,dslbreakeven=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].dslbreakeven);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,dslType=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].dslType);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,stopLoss=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].stopLoss);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,takeProfit=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].takeProfit);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,timedExit=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].timedExit);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,hourFilter=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].hourFilter);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,dayFilter=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].dayFilter);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Testing4.....initKnatuFile: InstanceID = %d,sysType=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID],kantuFielParams[index].sysType);
	*/
	trailingStop = kantuFielParams[index].trailingStop;
	AFMTL = kantuFielParams[index].AFMTL;
	timeFrame = kantuFielParams[index].timeFrame;
	dslbreakeven = kantuFielParams[index].dslbreakeven;
	dslType =  kantuFielParams[index].dslType;
	stopLoss = kantuFielParams[index].stopLoss;
	takeProfit = kantuFielParams[index].takeProfit;
	timedExit = kantuFielParams[index].timedExit;
	hourFilter = kantuFielParams[index].hourFilter;
	dayFilter = kantuFielParams[index].dayFilter;
	sysType = kantuFielParams[index].sysType;

	inputTypes1 = kantuFielParams[index].inputTypes1;
	inputTypes2 = kantuFielParams[index].inputTypes2;
	inputShifts1 = kantuFielParams[index].inputShifts1;
	inputShifts2 =kantuFielParams[index].inputShifts2;
	magnitudes = kantuFielParams[index].magnitudes;

	arraySize = kantuFielParams[index].arraySize;
		 
	
	if (timeFrame !=pParams->settings[TIMEFRAME]){
		pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"TIMEFRAME Variable not set properly in frontend for this system. It should be %d. Aborting execution.", (int)timeFrame);
		return(FALSE);
	}

    pParams->settings[TL_ATR_MULTIPLIER]              = trailingStop;
    pParams->settings[USE_AFMTL]                      = AFMTL;
	pParams->settings[DSL_BREAKEVEN_BARS]             = dslbreakeven;
	pParams->settings[DSL_TYPE]						  = dslType;
	pParams->settings[SL_ATR_MULTIPLIER]              = stopLoss;
	pParams->settings[TP_ATR_MULTIPLIER]              = takeProfit;
	pParams->settings[TIMED_EXIT_BARS]				  = timedExit;


	if (signalType != BUY && signalType != SELL){
		/*
		free(inputTypes1);
		free(inputTypes2);
		free(inputShifts1);
		free(inputShifts2);
		free(magnitudes);		
		*/
		return(FALSE);
	}

	isPatternTrue = TRUE;
	

	for (i=0; i < arraySize; i++){
				
		if (hourFilter != -1 && hour() != hourFilter)
		{
			isPatternTrue = FALSE;
			break;
		}

		if (dayFilter != -1 && dayOfWeek() != dayFilter)
		{
			isPatternTrue = FALSE;
			break;
		}

		if (signalType == BUY){

			if (ruleConverter(inputTypes1[i], inputShifts1[i], BUY)-ruleConverter(inputTypes2[i], inputShifts2[i], BUY) <= magnitudes[i]*atr )
				{
					isPatternTrue = FALSE;
					break;
				} 
		}

		if (signalType == SELL){

			if (ruleConverter(inputTypes1[i], inputShifts1[i], SELL)-ruleConverter(inputTypes2[i], inputShifts2[i], SELL) >= -magnitudes[i]*atr )
				{
					isPatternTrue = FALSE;
					break;
				} 
		}

		//pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"initKnatuFile: InstanceID = %d, inputTypes1 = %d, inputShifts1 =%d,inputTypes2=%d,inputShifts2=%d,magnitudes=%lf ", (int)pParams->settings[STRATEGY_INSTANCE_ID],inputTypes1[i],inputShifts1[i],inputTypes2[i],inputShifts2[i],magnitudes[i]);

	}

	if (signalType == BUY && sysType == SHORT_ONLY){
		isPatternTrue = FALSE;
	}

	if (signalType == SELL && sysType == LONG_ONLY){
		isPatternTrue = FALSE;
	}
	/*
	free(inputTypes1);
	free(inputTypes2);
	free(inputShifts1);
	free(inputShifts2);
	free(magnitudes);
	*/

	return(isPatternTrue);

}

int getKantuSignalML(StrategyParams* pParams, int systemFile){

	char fileName[250] = "";
	char tempPath[250] = "";
	char data[100] = "";
	char *ptr;
	double dslType = 0,
	       hourFilter=-1, 
           dayFilter=-1, 
           AFMTL=0,
	       dslbreakeven=0, 
           stopLoss=0, 
           takeProfit=0, 
           trailingStop=0, 
           timedExit=0;
    
	int *frontier;
    int *ml_algo_type;
    int *bars_used;
    int *learning_period;
	int i = 0;
	int arraySize = 0;
	int ensemblePrediction = 0;

	double timeFrame;
	char *strtokSave;
	FILE *fp;

	requestTempFileFolderPath(tempPath);

	sprintf(fileName, "%skantu_ml_%d.csv", tempPath, systemFile);

	fp = fopen( fileName, "r" );
	if(fp == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"KantuStrategies() Failed to open strategy file.");
	}

	arraySize = countLinesInCSV(fileName)-1;

	frontier          =  (int*)malloc((arraySize) * sizeof(int));
	ml_algo_type      =  (int*)malloc((arraySize) * sizeof(int));
	bars_used         =  (int*)malloc((arraySize) * sizeof(int));
	learning_period   =  (int*)malloc((arraySize) * sizeof(int));

    while ( fgets(data, 150, fp ) != NULL) {

		if (i == 0){

		ptr = strtok_r(data, ",", &strtokSave);
		sscanf(ptr,"%lf",&hourFilter);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&dayFilter);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&timedExit);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&stopLoss);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&takeProfit);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&AFMTL);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&trailingStop);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&timeFrame);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&dslbreakeven);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%lf",&dslType);
        ptr = strtok_r(NULL, ",", &strtokSave);

		} else {

		ptr = strtok_r(data, ",", &strtokSave);
		sscanf(ptr,"%d",&ml_algo_type[i-1]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%d",&frontier[i-1]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%d",&learning_period[i-1]);
		ptr = strtok_r(NULL, ",", &strtokSave);
		sscanf(ptr,"%d",&bars_used[i-1]);

		}

		i++;
	}

	fclose(fp);

	if (timeFrame !=pParams->settings[TIMEFRAME]){
		pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"TIMEFRAME Variable not set properly in frontend for this system. It should be %d. Aborting execution.", (int)timeFrame);
	}

    pParams->settings[DAY_FILTER_ML]                  = dayFilter;
    pParams->settings[TRADING_HOUR_ML]                 = hourFilter;
    pParams->settings[TL_ATR_MULTIPLIER_ML]           = trailingStop;
    pParams->settings[USE_AFMTL_ML]                   = AFMTL;
	pParams->settings[DSL_BREAKEVEN_BARS_ML]          = dslbreakeven;
	pParams->settings[DSL_TYPE_ML]					  = dslType;
	pParams->settings[SL_ATR_MULTIPLIER]              = stopLoss;
	pParams->settings[TP_ATR_MULTIPLIER]              = takeProfit;
	pParams->settings[TIMED_EXIT_BARS_ML]			  = timedExit;
    
	if ((hour() == (int)parameter(TRADING_HOUR_ML)) || ((int)parameter(TRADING_HOUR_ML) == -1)) {

		for(i=0;i<arraySize;i++){
			if (ml_algo_type[i] == LINEAR_REGRESSION){
				ensemblePrediction += LR_Prediction_i_simpleReturn_o_mlemse(learning_period[i], bars_used[i], frontier[i]);
			}

			if (ml_algo_type[i] == NEURAL_NETWORK){
				ensemblePrediction += NN_Prediction_i_simpleReturn_o_mlemse(learning_period[i], bars_used[i], frontier[i]);
			}

			if (ml_algo_type[i] == K_NEAREST_NEIGHBOR){
				ensemblePrediction += KNN_Prediction_i_simpleReturn_o_mlemse(learning_period[i], bars_used[i], frontier[i]);
			}
		}

	} else {
		free(frontier);
		free(learning_period);
		free(bars_used);
		free(ml_algo_type);
		return(0);
	}

	free(frontier);
	free(learning_period);
	free(bars_used);
	free(ml_algo_type);

	if (ensemblePrediction == arraySize) return(1);
	if (ensemblePrediction == 2*arraySize) return(2);

	return(0);
}


BOOL isLongEntrySignal(StrategyParams* pParams, int tradingStrategyUsed, int shift1Index, double atr){

	if (parameter(SELECT_KANTU_SYSTEM_FILE) != 0){

		return(getKantuSignal(pParams, BUY, (int)parameter(SELECT_KANTU_SYSTEM_FILE), atr));

	}

return FALSE ;
}

BOOL isShortEntrySignal(StrategyParams* pParams, int tradingStrategyUsed, int shift1Index, double atr){

	if (parameter(SELECT_KANTU_SYSTEM_FILE) != 0){

		return(getKantuSignal(pParams, SELL, (int)parameter(SELECT_KANTU_SYSTEM_FILE), atr));

	}

return FALSE ;
}
