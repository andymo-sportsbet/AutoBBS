/**
 * @file
 * @brief     Saving of user interface txt file.
 * @details   This code saves a file containing relevant UI information read by the front-ends.
 * 
 * @author    Daniel Fernandez (initial implementation)
 * @author    Morgan Doel (review and modifications)
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
#include "StrategyUserInterface.h"
#include "EasyTradeCWrapper.hpp"
#include "AsirikuyTime.h"

#include "Logging.h"

static char tempFilePath[MAX_FILE_PATH_CHARS] ;

AsirikuyReturnCode setTempFileFolderPath(char* tempPath)
{
		strcpy (tempFilePath,tempPath);
		strcat (tempFilePath, "/");
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"UI file saving folder set to : %s", tempFilePath);

		return SUCCESS;
}

AsirikuyReturnCode requestTempFileFolderPath(char* tempPath)
{
		strcpy (tempPath,tempFilePath);

		return SUCCESS;
}

AsirikuyReturnCode saveUserInterfaceValues(char* userInterfaceVariableNames[TOTAL_UI_VALUES], double userInterfaceValues[TOTAL_UI_VALUES], int userInterfaceElementsCount, int instanceID, BOOL isBackTesting)
{
  char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = ".ui" ;
	int n;
	FILE *fp;

	if(isBackTesting)
	{
    /* Don't save this file while testing because it's too slow. */
    return SUCCESS;
  }

    sprintf(instanceIDName, "%d", instanceID);
	strcat(buffer, tempFilePath);
	strcat(buffer, instanceIDName);
	strcat(buffer, extension);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"saveUserInterfaceValues() Saving UI variable file to : %s", buffer);
   
	fp = fopen(buffer,"w");
  if(fp == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"saveUserInterfaceValues() Failed to open UI variable file.");
    return NULL_POINTER;
  }

	for(n=0; n < userInterfaceElementsCount; n++) 
	{
		fprintf(fp, "%s, %lf\n", userInterfaceVariableNames[n], userInterfaceValues[n]);
	}

	fclose(fp);

	return SUCCESS;
}

AsirikuyReturnCode saveUserHeartBeat(int instanceID, BOOL isBackTesting)
{
    char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_heartBeat.hb" ;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	time_t rawtime;
    struct tm timeinfo;
    FILE *fp;

	/* This function is in the StrategyUserInterface.c file because 
	it's information is used to draw a part of the UI 
	(top right corner --  update time) */

	if(isBackTesting)
	{
    /* Don't save this file while testing because it's not necessary. */
    return SUCCESS;
    }

	time ( &rawtime );
    safe_gmtime(&timeinfo, rawtime);

    sprintf(instanceIDName, "%d", instanceID);
	strcat(buffer, tempFilePath);
	strcat(buffer, instanceIDName);
	strcat(buffer, extension);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"saveUserInterfaceValues() Saving tick heartbeat to : %s", buffer);
   
  fp = fopen(buffer,"w");
  if(fp == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"saveUserInterfaceValues() Failed to open heartbeat file.");
    return NULL_POINTER;
  }

		fprintf(fp, "%d\n", timeinfo.tm_hour);
		safe_timeString(timeString, rawtime);
		fprintf(fp, "%s", timeString);

	fclose(fp);

	return SUCCESS;
}

AsirikuyReturnCode savePredicatedWeeklyATR(char * pName, double predicatedWeeklyATR, double predicatedMaxWeeklyATR, BOOL isBackTesting)
{
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_weekly.txt";
	char timeString[MAX_TIME_STRING_SIZE] = "";
	time_t rawtime;
	struct tm timeinfo;
	FILE *fp;

	if (isBackTesting)
	{
		/* Don't save this file while testing because it's not necessary. */
		return SUCCESS;
	}

	time(&rawtime);
	safe_gmtime(&timeinfo, rawtime);

	strcat(buffer, tempFilePath);
	strcat(buffer, pName);
	strcat(buffer, extension);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"saveUserInterfaceValues() Saving weekly ATR to : %s", buffer);

	fp = fopen(buffer, "w");
	if (fp == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"saveUserInterfaceValues() Failed to open weekly ATR file.");
		return NULL_POINTER;
	}

	fprintf(fp, "%lf\n", predicatedWeeklyATR);
	fprintf(fp, "%lf\n", predicatedMaxWeeklyATR);
	


	fclose(fp);

	return SUCCESS;
}

AsirikuyReturnCode saveRateFile(int instanceID, int rate,BOOL isBackTesting)
{
	char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_rate.txt";
	FILE *fp;

	/* This function is in the StrategyUserInterface.c file because
	it's information is used to draw a part of the UI
	(top right corner --  update time) */

	if (isBackTesting)
	{
		/* Don't save this file while testing because it's not necessary. */
		return SUCCESS;
	}

	sprintf(instanceIDName, "%d", instanceID);
	strcat(buffer, tempFilePath);
	strcat(buffer, instanceIDName);
	strcat(buffer, extension);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"saveRateFile() %s", buffer);

	fp = fopen(buffer, "w");
	if (fp == NULL)
	{
		return SUCCESS;
	}

	fprintf(fp, "%d\n", rate);

	fclose(fp);

	return SUCCESS;
}

int readWeeklyATRFile(char * pName,double *pPredictWeeklyATR,double *pPredictWeeklyMaxATR, BOOL isBackTesting)
{	
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_weekly.txt";
	char line[1024] = "";
	FILE *fp;
	int rateErrorTimes = -1;

	/* This function is in the StrategyUserInterface.c file because
	it's information is used to draw a part of the UI
	(top right corner --  update time) */

	if (isBackTesting)
	{
		/* Don't save this file while testing because it's not necessary. */
		return 0;
	}

	strcat(buffer, tempFilePath);
	strcat(buffer, pName);
	strcat(buffer, extension);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"readWeeklyATRFile() %s", buffer);

	fp = fopen(buffer, "r");
	if (fp == NULL)
	{
		return rateErrorTimes;
	}

	fgets(line, 1024, fp);
	*pPredictWeeklyATR = atof(line);
	fgets(line, 1024, fp);
	*pPredictWeeklyMaxATR = atof(line);
	
	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"readWeeklyATRFile() pPredictWeeklyATR= %f,pPredictWeeklyMaxATR=%f", *pPredictWeeklyATR, *pPredictWeeklyMaxATR);

	fclose(fp);

	return 1;
	
}

int readRateFile(int instanceID, BOOL isBackTesting)
{
	char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_rate.txt";
	char line[1024] = "";
	FILE *fp;
	int rateErrorTimes = -1;

	/* This function is in the StrategyUserInterface.c file because
	it's information is used to draw a part of the UI
	(top right corner --  update time) */

	if (isBackTesting)
	{
		/* Don't save this file while testing because it's not necessary. */
		return rateErrorTimes;
	}

	sprintf(instanceIDName, "%d", instanceID);
	strcat(buffer, tempFilePath);
	strcat(buffer, instanceIDName);
	strcat(buffer, extension);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"readRateFile() %s", buffer);

	fp = fopen(buffer, "r");
	if (fp == NULL)
	{		
		return rateErrorTimes;
	}

	while (fgets(line, 1024, fp)) {
		rateErrorTimes = atoi(line);
	}
	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"readRateFile() rateTimes= %d", rateErrorTimes);

	fclose(fp);

	return rateErrorTimes;
}

//这里包括非农，和美联储议息会议的时间
//这里只有日期，没有时间。
int readXAUUSDKeyNewsDateFile(time_t *pKeyDates)
{
//	char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char fileName[] = "XAUUSDKeyNewsDate.txt";
	char line[1024] = "";
	FILE *fp;
	time_t now;
	int i = 0;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	struct tm timeInfo1;
	
	/* This function is in the StrategyUserInterface.c file because
	it's information is used to draw a part of the UI
	(top right corner --  update time) */
	
	strcat(buffer, tempFilePath);	
	strcat(buffer, fileName);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"readXAUUSDKeyNewsDateFile() %s", buffer);

	fp = fopen(buffer, "r");
	if (fp == NULL)
	{
		return -1;
	}

	now = time(NULL);

	while (fgets(line, 1024, fp)) {
		
		*(pKeyDates+i) = curl_getdate(line, &now);
		safe_gmtime(&timeInfo1, *(pKeyDates + i));
		safe_timeString(timeString, *(pKeyDates + i));
		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"readXAUUSDKeyNewsDateFile() KeyDate= %d", timeString);

		i++;		
	}

	fclose(fp);

	return 0;
}


AsirikuyReturnCode saveTradingInfo(int instanceID, Order_Info * pOrderInfo,BOOL isBackTesting)
{
	char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_OrderInfo.txt";
	char timeString[MAX_TIME_STRING_SIZE] = "";		
	FILE *fp;

	if (isBackTesting)
	{
		return SUCCESS;
	}


	sprintf(instanceIDName, "%d", instanceID);
	strcat(buffer, tempFilePath);
	strcat(buffer, instanceIDName);
	strcat(buffer, extension);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"saveTradingInfo() Saving trading order info to : %s", buffer);

	fp = fopen(buffer, "w");
	if (fp == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"saveTradingInfo() Failed to open trading order info file.");
		return NULL_POINTER;
	}

	fprintf(fp, "%f\n", pOrderInfo->high);
	fprintf(fp, "%f\n", pOrderInfo->low);
	fprintf(fp, "%d\n", pOrderInfo->orderNumber);
	fprintf(fp, "%d\n", pOrderInfo->isRetreat);
	fprintf(fp, "%d\n", pOrderInfo->side);
	fprintf(fp, "%d\n", pOrderInfo->exitSignal);
	fprintf(fp, "%f", pOrderInfo->risk);

	fclose(fp);

	return SUCCESS;
}

int readTradingInfo(int instanceID, Order_Info *pOrderInfo)
{
	char instanceIDName[TOTAL_UI_VALUES];
	char buffer[MAX_FILE_PATH_CHARS] = "";
	char extension[] = "_OrderInfo.txt";
	char line[1024] = "";
	FILE *fp;
	time_t now;
	int i = 0;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	//struct tm timeInfo1;



	sprintf(instanceIDName, "%d", instanceID);
	strcat(buffer, tempFilePath);
	strcat(buffer, instanceIDName);
	strcat(buffer, extension);
	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"readTradingInfo() %s", buffer);

	fp = fopen(buffer, "r");
	if (fp == NULL)
	{
		return -1;
	}

	now = time(NULL);

	fgets(line, 1024, fp);
	pOrderInfo->high= atof(line);
	pOrderInfo->low = atof(line);
	pOrderInfo->orderNumber = atoi(line);
	pOrderInfo->isRetreat = atoi(line);
	pOrderInfo->side = atoi(line);
	pOrderInfo->exitSignal = atoi(line);
	pOrderInfo->risk = atof(line);
	
	fclose(fp);

	return 0;
}