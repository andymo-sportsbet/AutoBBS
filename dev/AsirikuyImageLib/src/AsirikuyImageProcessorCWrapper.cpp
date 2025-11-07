/**
 * @file
 * @brief     Image processing functions.
 * 
 * @author    Daniel Fernandez (Initial implementation)
 * @author    Morgan Doel (Ported the code to the F4 framework)
 * @version   F4.x.x
 * @date      2013
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
#include <ta_func.h>
#include <chartdir.h>
#include <bchartdir.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include "AsirikuyTime.h"
#include "AsirikuyImageProcessorCWrapper.hpp"
#include "StrategyUserInterface.h"
#include "EasyTradeCWrapper.hpp"

AsirikuyReturnCode plotWorstCaseTrack(StrategyParams* pParams, 
										 double* tradeResults, 
										 int* tradeOpeningTime, 
										 double regressionSlope,
										 double initialBalance, 
										 double standardDeviationOfResiduals,
										 double finalBalanceValue,
										 double initialTime,
										 int tradeNumber,
										 int width, 
										 int height)
	{
		
		

		char** labels;

		char tempPath[MAX_FILE_PATH_CHARS];
		char saveToPath[MAX_FILE_PATH_CHARS];


		requestTempFileFolderPath(tempPath);

		double* currentBalance = (double*)malloc(tradeNumber  * sizeof(double));
		double* linearPlot = (double*)malloc(tradeNumber  * sizeof(double));
		double* standardDeviationPlot = (double*)malloc(tradeNumber  * sizeof(double));
		double weeksPassed;

		labels = (char**)malloc((tradeNumber) * sizeof(char*));

		for (int i = 0; i < tradeNumber; i++)
		{
			weeksPassed = (tradeOpeningTime[i]-tradeOpeningTime[0])/604800;
			linearPlot[i] =  regressionSlope*(tradeOpeningTime[i]-initialTime)+initialBalance; 
			standardDeviationPlot[i] =  regressionSlope*(tradeOpeningTime[i]-initialTime)+initialBalance-(standardDeviationOfResiduals*3.5+standardDeviationOfResiduals*weeksPassed*0.015);
			
			if (i>0)
				currentBalance[i] = currentBalance[i-1] + tradeResults[i]*initialBalance*(1/pParams->settings[ACCOUNT_RISK_PERCENT]);
			else
				currentBalance[i] = finalBalanceValue;
			
			labels[i] = (char*)malloc((MAX_TIME_STRING_SIZE) * sizeof(char)); 

			safe_timeString(labels[i], tradeOpeningTime[i]);
		}


		XYChart *c = new XYChart(width, height);

		c->setPlotArea(50, 30, width-100, height-150);
  
		c->addTitle("Worst case track");
		c->yAxis()->setTitle("Aroon Value");
		c->xAxis()->setTitle("Time");
		c->xAxis()->setLabels(StringArray(labels, tradeNumber))->setFontAngle(90);
		c->xAxis()->setLabelStep(25);

		LineLayer *layer1 = c->addLineLayer( DoubleArray(linearPlot,  tradeNumber), 0x008000, "Linear regression");
		LineLayer *layer2 = c->addLineLayer( DoubleArray(standardDeviationPlot,  tradeNumber), 0x4B0082, "3.5 x Standard Deviation Line");
		LineLayer *layer3 = c->addLineLayer( DoubleArray(currentBalance,  tradeNumber), 0xff0000, "Live trading curve");
  
		layer1 ->setLineWidth(2);
		layer2 ->setLineWidth(2);
		layer3 ->setLineWidth(2);

		c->setBorder(0xFFFFFF);
	
		sprintf(saveToPath, "%s%d%s", tempPath, (int)pParams->settings[STRATEGY_INSTANCE_ID], "_WorstCaseTrack.png");
		c->makeChart(saveToPath);

		delete c;

		for (int i = 0; i < tradeNumber; i++){
			free(labels[i]);
		}

		free(labels);
		free(linearPlot);
		free(standardDeviationPlot);
		free(currentBalance);

		return SUCCESS;
	}

AsirikuyReturnCode plotAroonOsc(StrategyParams* pParams, int period, int barsToPlot, int width, int height)
	{
		
		
		/* create an array of array of chars, this is simply an array of strings to save the labels */
		char** labels;
		int outBegIdx;
		int outNBElement;
		double Aroonslow;
		int shift1 = pParams->ratesBuffers->rates[2].info.arraySize - 1;
		/* create a tempPath variable that will hold the location of the temp folder path */
		char tempPath[MAX_FILE_PATH_CHARS];
		char saveToPath[MAX_FILE_PATH_CHARS];

		/* get the temp file path into the tempPath string */
		requestTempFileFolderPath(tempPath);

		/* create an array to hold values for the rsi */
		double* aroon = (double*)malloc(barsToPlot  * sizeof(double));

		/* allocate memory to the labels array*/
		labels = (char**)malloc((barsToPlot) * sizeof(char*));

		/* now we are going to populate the Aroon Oscillator and the labels */
		for (int i = 0; i < barsToPlot; i++)
		{
			TA_AROONOSC(shift1-i, shift1-i, pParams->ratesBuffers->rates[2].high, pParams->ratesBuffers->rates[2].low, period, &outBegIdx, &outNBElement, &Aroonslow);
			aroon[i] = Aroonslow; 
			labels[i] = (char*)malloc((MAX_TIME_STRING_SIZE) * sizeof(char)); 
			safe_timeString(labels[i], openTime(i));
		}

		/* create the Chart director canvas, called "c" */
		XYChart *c = new XYChart(width, height);

		/* set the plot area */
		c->setPlotArea(50, 30, width-100, height-150);
  
		/* add titles and labels */
		c->addTitle("Aroon Chart");
		c->yAxis()->setTitle("Aroon Value");
		c->xAxis()->setTitle("Time");
		c->xAxis()->setLabels(StringArray(labels, barsToPlot))->setFontAngle(90);

		/* add the line plot for the Oscillator */
		LineLayer *layer1 = c->addLineLayer( DoubleArray(aroon,  barsToPlot), 0xff0000);
  
		/* set the line width to 2 (thicker line) */
		layer1 ->setLineWidth(2);

		/* set border color */
		c->setBorder(0xFFFFFF);

		/* save the chart to the temp folder location (experts/files) plus RSI_plot.png */
		sprintf(saveToPath, "%s%d%s", tempPath, period,"_Aroon_plot.png");
		c->makeChart(saveToPath);


		/* free all objects to prevent memory leaks */
		delete c;
		
		for (int i = 0; i < barsToPlot; i++){
			free(labels[i]);
		}

		free(labels);
		free(aroon);

		return SUCCESS;
	}

AsirikuyReturnCode plotRSI(int period, int barsToPlot, int width, int height)
{
  /* create an array of array of chars, this is simply an array of strings to save the labels */
  char** labels;

  /* create a tempPath variable that will hold the location of the temp folder path */
  char tempPath[MAX_FILE_PATH_CHARS];

  /* get the temp file path into the tempPath string */
  requestTempFileFolderPath(tempPath);

  /* create an array to hold values for the rsi */
  double* rsi = (double*)malloc(barsToPlot  * sizeof(double));

  /* allocate memory to the labels array*/
  labels = (char**)malloc((barsToPlot) * sizeof(char*));

  /* now we are going to populate the RSI and the labels */
  for (int i = 0; i < barsToPlot; i++)
  {
  rsi[i] =  iRSI(PRIMARY_RATES, period, i);
  labels[i] = (char*)malloc((MAX_TIME_STRING_SIZE) * sizeof(char)); 
  safe_timeString(labels[i], openTime(i));
  }

  /* create the Chart director canvas, called "c" */
  XYChart *c = new XYChart(width, height);

  /* set the plot area */
  c->setPlotArea(50, 30, width-100, height-150);
  
  /* add titles and labels */
  c->addTitle("RSI Chart");
  c->yAxis()->setTitle("RSI Value");
  c->xAxis()->setTitle("Time");
  c->xAxis()->setLabels(StringArray(labels, barsToPlot))->setFontAngle(90);

  /* add the line plot for the RSI */
  LineLayer *layer1 = c->addLineLayer( DoubleArray(rsi,  barsToPlot), 0xff0000);
  
  /* set the line width to 2 (thicker line) */
  layer1 ->setLineWidth(2);

  /* set border color */
  c->setBorder(0xFFFFFF);

  /* save the chart to the temp folder location (experts/files) plus RSI_plot.png */
  c->makeChart(strcat(tempPath, "RSI_plot.png"));

  /* free all objects to prevent memory leaks */
  delete c;
  
		for (int i = 0; i < barsToPlot; i++){
			free(labels[i]);
		}

  free(labels);
  free(rsi);

  return SUCCESS;
}

AsirikuyReturnCode chartAllInputData(int ratesArrayIndex, int width, int height)
{
  char** labels;
  char tempPath[MAX_FILE_PATH_CHARS];
  char instanceIDName[MAX_FILE_PATH_CHARS];
  char extension[] = "_OHLC.png" ;
  char buffer[MAX_FILE_PATH_CHARS] = "";

  StrategyParams* pParams = getParams();

  int barNumber = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize;

  double* open = (double*)malloc(barNumber  * sizeof(double));
  double* high = (double*)malloc(barNumber  * sizeof(double));
  double* low = (double*)malloc(barNumber  * sizeof(double));
  double* close = (double*)malloc(barNumber * sizeof(double));

  requestTempFileFolderPath(tempPath);

  sprintf(instanceIDName, "%d", (int)parameter(STRATEGY_INSTANCE_ID));
  strcat(buffer, tempPath);
  strcat(buffer, instanceIDName);
  strcat(buffer, extension);

  labels = (char**)malloc((barNumber+10) * sizeof(char*));

  for (int i = 0; i < barNumber; i++)
  {
  open[i] =  pParams->ratesBuffers->rates[ratesArrayIndex].open[i];
  high[i] =  pParams->ratesBuffers->rates[ratesArrayIndex].high[i];
  low[i]  =   pParams->ratesBuffers->rates[ratesArrayIndex].low[i];
  close[i] = pParams->ratesBuffers->rates[ratesArrayIndex].close[i];
  labels[i] = (char*)malloc((MAX_TIME_STRING_SIZE) * sizeof(char)); 
  safe_timeString(labels[i], pParams->ratesBuffers->rates[ratesArrayIndex].time[i]);
  }

  XYChart *c = new XYChart(width, height);

  c->setPlotArea(50, 30, width-100, height-150);
  
  c->addTitle("All Data Chart");
  c->yAxis()->setTitle("Bid Price");
  CandleStickLayer *layer1 = c->addCandleStickLayer( DoubleArray(high,  barNumber), 
													 DoubleArray(low,  barNumber),
													 DoubleArray(open,  barNumber),
													 DoubleArray(close,  barNumber),
													 0x00ff00, 0xff0000);
  c->xAxis()->setTitle("Time");
  c->xAxis()->setLabels(StringArray(labels, barNumber))->setFontAngle(90);

  layer1 ->setLineWidth(2);

  c->setBorder(0xFFFFFF);

  c->makeChart(buffer);

  delete c;
  
		for (int i = 0; i < barNumber; i++){
			free(labels[i]);
		}

  free(labels);
  free(open);
  free(high);
  free(low);
  free(close);

  return SUCCESS;
}

AsirikuyReturnCode drawChartForNN_tester(double *open, double *high, double *low, double *close, int tradeNumber, char* buffer, int canvasSize, double upperLimit,  double lowerLimit, int isAutoScaling, int useHighLow)
{

  XYChart *c = new XYChart(canvasSize, canvasSize);

  c->setTransparentColor(-1);

  c->setPlotArea(1, 1, canvasSize-1, canvasSize-1, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF)->setGridColor(0xFFFFFF, 0xFFFFFF);

  if (useHighLow == 1)
  {
    LineLayer *layer1 = c->addLineLayer( DoubleArray(high,  tradeNumber), 000000, "", 0);
    LineLayer *layer2 = c->addLineLayer( DoubleArray(low,  tradeNumber), 000000, "", 0);
  }

  LineLayer *layer3 = c->addLineLayer( DoubleArray(open,  tradeNumber), 000000, "", 0);
  LineLayer *layer4 = c->addLineLayer( DoubleArray(close,  tradeNumber), 000000, "", 0);

  if (isAutoScaling == 1)
  c->yAxis()->setLinearScale(lowerLimit, upperLimit) ;

  c->setBorder(0xFFFFFF);

  c->makeChart(buffer);

  delete c;

  return SUCCESS;
}

AsirikuyReturnCode drawHistogramForNN_tester(double *probabilities, char* buffer)
{

  char *labels[2] = {
    "negative",
	"positive"

  };

  XYChart *c = new XYChart(640, 468);

  c->setPlotArea(1, 1, 580, 400, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF)->setGridColor(0xFFFFFF, 0xFFFFFF);

  BarLayer *layer3 = c->addBarLayer( DoubleArray(probabilities,  2));

  c->xAxis()->setLabels(StringArray(labels, 2))->setFontAngle(45);

  c->addTitle("Histogram");

  c->xAxis()->setTitle("Class");

  c->setYAxisOnRight(true);

  c->yAxis()->setTitle("Probability");

  c->makeChart(buffer);

  delete c;

  return SUCCESS;
}
