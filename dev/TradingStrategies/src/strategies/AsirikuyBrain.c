/**
 * @file
 * @brief     The Asirikuy Brain trading system.
 * @details   Contains three NN training systems put together in a committee. Paqarin, Sunqu and Tapuy.
 * 
 * @author    Daniel Fernandez (Original ideas, initial implementations, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported Sunqu to the F4 framework)
 * @version   F4.x.x
 * @date      2014
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

#include <doublefann.h>
#include <fann_train.h>
#include <ta_libc.h>
#include <sys/stat.h>

#include "AsirikuyStrategies.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "OrderSignals.h"
#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "AsirikuyImageProcessorCWrapper.hpp"
#include "IL/il.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP TRUE

typedef enum additionalSettings_t
{
  TRAINING_SETS_PER_EPOCH_PAQARIN = ADDITIONAL_PARAM_1,
  TRAINING_EPOCHS_COUNT_PAQARIN   = ADDITIONAL_PARAM_2,
  INPUT_BARS_PAQARIN             = ADDITIONAL_PARAM_3,
  COMMITTEE_SIZE_PAQARIN          = ADDITIONAL_PARAM_4,
  TRAINING_SETS_PER_EPOCH_SUNQU	   = ADDITIONAL_PARAM_5,
  TRAINING_EPOCHS_COUNT_SUNQU	   = ADDITIONAL_PARAM_6,
  INPUT_BARS_SUNQU				   = ADDITIONAL_PARAM_7,
  COMMITTEE_SIZE_SUNQU			   = ADDITIONAL_PARAM_8,
  TRAINING_SETS_PER_EPOCH_TAPUY    = ADDITIONAL_PARAM_9,
  TRAINING_EPOCHS_COUNT_TAPUY      = ADDITIONAL_PARAM_10,
  INPUT_BARS_TAPUY                 = ADDITIONAL_PARAM_11,
  CANVAS_SIZE_TAPUY                = ADDITIONAL_PARAM_12,
  SCALING_MODE_TAPUY               = ADDITIONAL_PARAM_13,
  SAVE_HISTOGRAM_TAPUY             = ADDITIONAL_PARAM_14,
  USE_HIGH_LOW_TAPUY			  = ADDITIONAL_PARAM_15,
  USE_PAQARIN			          = ADDITIONAL_PARAM_16,
  USE_SUNQU			              = ADDITIONAL_PARAM_17,
  USE_TAPUY			              = ADDITIONAL_PARAM_18,
  TRADE_ON_HOUR		              = ADDITIONAL_PARAM_19
} AdditionalSettings;

typedef enum NNTrainingResult_t
{
  NN_TRAINING_FAILURE = 0, 
  NN_TRAINING_SUCCESS = 1
} NNTrainingResult;

typedef enum NNCommitteeResult_t
{
  PREDICTION_NONE          = -1,
  PREDICTION_ALL_UP_MOVE   =  0,
  PREDICTION_ALL_DOWN_MOVE =  1,
  PREDICTION_DISAGREEMENT  =  2,
} NNCommitteeResult;

#define NN_TOTAL_LAYERS_PAQARIN		3
#define NN_OUTPUTS_PAQARIN			4

#define NN_TOTAL_LAYERS_SUNQU		3
#define NN_OUTPUTS_SUNQU			1

#define NN_TOTAL_LAYERS_TAPUY		3
#define NN_OUTPUTS_TAPUY			2
#define NN_INPUT_BARS_TAPUY			10

static const float     NN_CONNECTION_RATE   = 1;
static const float     NN_LEARNING_RATE     = 0.7F;
static const float     NN_LEARNING_MOMENTUM = 0;
static const fann_type MIN_WEIGHT           = -0.5;
static const fann_type MAX_WEIGHT           = 0.5;
static const fann_type STEEPNESS_HIDDEN     = 0.5;
static const fann_type STEEPNESS_OUTPUT     = 0.5;

static void trainNeuralNetworksSunqu(StrategyParams* pParams, struct fann** ppNeuralNetworks, double* pInputs)
{
  int i, k, m, n;
  double  output;

  for(i = 0; i < pParams->settings[COMMITTEE_SIZE_SUNQU]; i++)
  {
    unsigned int layers[NN_TOTAL_LAYERS_SUNQU];
    layers[0] = (unsigned int)pParams->settings[INPUT_BARS_SUNQU];
    layers[1] = (unsigned int)pParams->settings[INPUT_BARS_SUNQU];
    layers[2] = NN_OUTPUTS_SUNQU;
    ppNeuralNetworks[i] = fann_create_standard_array(NN_TOTAL_LAYERS_SUNQU, layers);
    fann_randomize_weights(ppNeuralNetworks[i], MIN_WEIGHT, MAX_WEIGHT);
    fann_set_learning_rate(ppNeuralNetworks[i], NN_LEARNING_RATE);
    fann_set_learning_momentum(ppNeuralNetworks[i], NN_LEARNING_MOMENTUM);
    fann_set_activation_steepness_hidden(ppNeuralNetworks[i], STEEPNESS_HIDDEN);
    fann_set_activation_steepness_output(ppNeuralNetworks[i], STEEPNESS_OUTPUT);
    fann_set_activation_function_hidden(ppNeuralNetworks[i], FANN_SIGMOID);
    fann_set_activation_function_output(ppNeuralNetworks[i], FANN_SIGMOID);
    fann_set_training_algorithm(ppNeuralNetworks[i], FANN_TRAIN_RPROP);
    fann_reset_MSE(ppNeuralNetworks[i]);

    for(k = 0; k < pParams->settings[TRAINING_EPOCHS_COUNT_SUNQU]; k++)
    {
      for(m = pParams->ratesBuffers->rates[1].info.arraySize - (int)pParams->settings[TRAINING_SETS_PER_EPOCH_SUNQU]; m < pParams->ratesBuffers->rates[1].info.arraySize; m++)
      {
        for(n = 0; n < (int)pParams->settings[INPUT_BARS_SUNQU]; n++)
        {
          pInputs[n] = pParams->ratesBuffers->rates[1].close[m + n - (int)pParams->settings[INPUT_BARS_SUNQU]];
        }

        if(m == pParams->ratesBuffers->rates[1].info.arraySize - 1)
        {
          output = pParams->ratesBuffers->rates[1].open[m];
        }
        else
        {
          output = pParams->ratesBuffers->rates[1].close[m];
        }

        /* output must be between 0 and 1. A simple divide is used instead of normalizing because the neural network works better if the values aren't close to 1. */
        if(strstr(pParams->tradeSymbol, "JPY") == NULL)
        {
          output /= 10;
        }
        else
        {
          output /= 1000;
        }
          
        /* Train the neural network */
        fann_train(ppNeuralNetworks[i], pInputs, &output);
      }
    }
  }
}

static int runNeuralNetworksSunqu(StrategyParams* pParams, struct fann** ppNeuralNetworks, double* pInputs)
{
  int i, j, predictionScore = 0;

  for(i = 0; i < pParams->settings[COMMITTEE_SIZE_SUNQU]; i++)
  {
    double firstOutput, secondOutput;

    for(j = 0; j < (int)pParams->settings[INPUT_BARS_SUNQU]; j++)
    {
      pInputs[j] = pParams->ratesBuffers->rates[1].close[pParams->ratesBuffers->rates[1].info.arraySize - j - 2];
    }

    /* Run the neural network */
    firstOutput = *fann_run(ppNeuralNetworks[i], pInputs);
    for(j = 0; j < (int)pParams->settings[INPUT_BARS_SUNQU]; j++)
    {
      if(j == 0)
      {
        pInputs[j] = pParams->ratesBuffers->rates[1].open[pParams->ratesBuffers->rates[1].info.arraySize - j - 1];
      }
      else
      {
        pInputs[j] = pParams->ratesBuffers->rates[1].close[pParams->ratesBuffers->rates[1].info.arraySize - j - 1];
      }
    }

    /* Run the neural network again */
    secondOutput = *fann_run(ppNeuralNetworks[i], pInputs);
    if(firstOutput < secondOutput)
    {
      predictionScore--;
    }
    else if(firstOutput > secondOutput)
    {
      predictionScore++;
    }

    /* Destroy the neural network */
    fann_destroy(ppNeuralNetworks[i]);
  }

  return predictionScore;
}

static NNCommitteeResult getPredictionSunqu(StrategyParams* pParams)
{
  NNCommitteeResult prediction = PREDICTION_NONE;
  double*           inputs = (double*)malloc((int)pParams->settings[INPUT_BARS_SUNQU] * sizeof(double)), unroundedScoreThreshold = 0.01 * 100 * pParams->settings[COMMITTEE_SIZE_SUNQU];
  int               predictionScore = 0, scoreThreshold = (int)((unroundedScoreThreshold > (floor(unroundedScoreThreshold)+0.5f)) ? ceil(unroundedScoreThreshold) : floor(unroundedScoreThreshold));
  struct fann**     neuralNetworks = (struct fann**)malloc((int)pParams->settings[COMMITTEE_SIZE_SUNQU] * sizeof(struct fann*));
  
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Sunqu training start");
  
  trainNeuralNetworksSunqu(pParams, neuralNetworks, inputs);

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Sunqu training end");

  predictionScore = runNeuralNetworksSunqu(pParams, neuralNetworks, inputs);

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Sunqu prediction end");
  
  if(predictionScore >= scoreThreshold)
  {
    prediction = PREDICTION_ALL_UP_MOVE;
  }
  else if(predictionScore <= -scoreThreshold)
  {
    prediction = PREDICTION_ALL_DOWN_MOVE;
  }
  else
  {
    prediction = PREDICTION_DISAGREEMENT;
  }

  free(neuralNetworks);
  free(inputs);

  return prediction;
}


static void findHighestAndLowest(StrategyParams* pParams, double* highestOutput, double* lowestOutput)
{
	int m;
	double highest = 0;
	double lowest = 10000;

	 for(m = pParams->ratesBuffers->rates[1].info.arraySize - (int)pParams->settings[TRAINING_SETS_PER_EPOCH_TAPUY]- (int)pParams->settings[INPUT_BARS_TAPUY]; m < pParams->ratesBuffers->rates[1].info.arraySize-1; m++)
     {
		 if (pParams->ratesBuffers->rates[1].open[m] > highest) 
		 {
			 highest = pParams->ratesBuffers->rates[1].open[m];
		 }

		 if (pParams->ratesBuffers->rates[1].close[m] > highest) 
		 {
			 highest = pParams->ratesBuffers->rates[1].close[m];
		 }

		 if (pParams->ratesBuffers->rates[1].open[m] < lowest) 
		 {
			 lowest = pParams->ratesBuffers->rates[1].open[m];
		 }

		 if (pParams->ratesBuffers->rates[1].close[m] < lowest) 
		 {
			 lowest = pParams->ratesBuffers->rates[1].close[m];
		 }

	 }

	 *highestOutput = highest;
	 *lowestOutput = lowest;
}


static void trainNeuralNetworksTapuy(StrategyParams* pParams, struct fann* pNeuralNetwork)
{
 
  int k, m, n, h, w, outBegIdx, outNBElement;
  double  body, atr;
  double* open = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY]  * sizeof(double));
  double* high = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY]  * sizeof(double));
  double* low = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY]  * sizeof(double));
  double* close = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY] * sizeof(double));
  double output[2];
  ILuint imgID = 0;
  ILubyte * bytes;
  ILuint width,height;
  ILubyte *Lump;
  ILuint Size;
  FILE *File;
  char tempPath[MAX_FILE_PATH_CHARS];
  char extension[] = ".png" ;
  char instanceIDName[TOTAL_UI_VALUES];
  char buffer[MAX_FILE_PATH_CHARS] = "";
  double periodHigh, periodLow;
  double *pInputs = (double*)malloc((int)pParams->settings[CANVAS_SIZE_TAPUY]*(int)pParams->settings[CANVAS_SIZE_TAPUY] * sizeof(double));


  requestTempFileFolderPath(tempPath);

   pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Tapuy train start");

  sprintf(instanceIDName,"%d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
  strcat(buffer, tempPath);
  strcat(buffer, instanceIDName);
  strcat(buffer, extension);

  if ((int)pParams->settings[SCALING_MODE_TAPUY] == 1)
  findHighestAndLowest(pParams, &periodHigh, &periodLow);

    fann_randomize_weights(pNeuralNetwork, MIN_WEIGHT, MAX_WEIGHT);
    fann_set_learning_rate(pNeuralNetwork, NN_LEARNING_RATE);
    fann_set_learning_momentum(pNeuralNetwork, NN_LEARNING_MOMENTUM);
    fann_set_activation_steepness_hidden(pNeuralNetwork, STEEPNESS_HIDDEN);
    fann_set_activation_steepness_output(pNeuralNetwork, STEEPNESS_OUTPUT);
    fann_set_activation_function_hidden(pNeuralNetwork, FANN_SIGMOID);
    fann_set_activation_function_output(pNeuralNetwork, FANN_SIGMOID);
    fann_set_training_algorithm(pNeuralNetwork, FANN_TRAIN_RPROP);
    fann_reset_MSE(pNeuralNetwork);

    for(k = 0; k < pParams->settings[TRAINING_EPOCHS_COUNT_TAPUY]; k++)
    {

		for(m = pParams->ratesBuffers->rates[1].info.arraySize - (int)pParams->settings[TRAINING_SETS_PER_EPOCH_TAPUY]; m < pParams->ratesBuffers->rates[1].info.arraySize-1; m++)
      {


        TA_ATR(m, m, pParams->ratesBuffers->rates[1].high, pParams->ratesBuffers->rates[1].low, pParams->ratesBuffers->rates[1].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);

		atr *= 24;

		for(n = 0; n < (int)pParams->settings[INPUT_BARS_TAPUY] ; n++)
        {
          open[n]  = pParams->ratesBuffers->rates[1].open[m - n - 1];

		  if( (int)pParams->settings[USE_HIGH_LOW_TAPUY] == 1)
		  {
		    high[n]  = pParams->ratesBuffers->rates[1].high[m - n - 1];
		    low[n]   = pParams->ratesBuffers->rates[1].low[m - n - 1];
		  }

		  close[n] = pParams->ratesBuffers->rates[1].close[m - n - 1];
        }

		drawChartForNN_tester(open, high, low, close, (int)pParams->settings[INPUT_BARS_TAPUY], buffer, (int)pParams->settings[CANVAS_SIZE_TAPUY], periodHigh, periodLow, (int)pParams->settings[SCALING_MODE_TAPUY], (int)pParams->settings[USE_HIGH_LOW_TAPUY]);

		ilGenImages( 1, &imgID );
        ilBindImage( imgID );

		File = fopen(buffer, "rb");
		fseek(File, 0, SEEK_END);
		Size = ftell(File);

		Lump = (ILubyte*)malloc(Size);
		fseek(File, 0, SEEK_SET);
		fread(Lump, 1, Size, File);
		fclose(File);

		ilLoadL(IL_PNG, Lump, Size);

		bytes = ilGetData();

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);	

		n = 0;

		for (h = 0; h < (int)height; h++)
		{
			for (w = 0; w < (int)width; w++)
			{
				pInputs[n] = bytes[(h*width + w)*4 + 0]; 
				n++ ;
			}
		}

		free(Lump);
		ilDeleteImages( 1, &imgID );

		output[0] = 0;
		output[1] = 0;

		body = pParams->ratesBuffers->rates[1].open[m]-pParams->ratesBuffers->rates[1].open[m-1] ;

		if (body < 0)
          output[0] =  1;

		if (body > 0)
          output[1] =  1;

        /* Train the neural network */
        fann_train(pNeuralNetwork, pInputs, output);
      }

    }

	
	free(open);
	free(high);
	free(low);
	free(close);
	free(pInputs);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Tapuy train end");

}

static double runNeuralNetworksTapuy(StrategyParams* pParams, struct fann* pNeuralNetwork)
{

  int j, h, w;
  double* output;
  double sumPositive = 0, sumNegative = 0;
  double predictionScore = 0;
  double* open = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY]  * sizeof(double));
  double* high = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY]  * sizeof(double));
  double* low = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY]  * sizeof(double));
  double* close = (double*)malloc((int)pParams->settings[INPUT_BARS_TAPUY] * sizeof(double));
  ILuint imgID = 0;
  ILubyte * bytes;
  ILuint width,height;
  ILubyte *Lump;
  ILuint Size;
  FILE *File;
  char tempPath[MAX_FILE_PATH_CHARS];
  char extension[] = ".png" ;
  char instanceIDName[TOTAL_UI_VALUES];
  char buffer[MAX_FILE_PATH_CHARS] = "";
  char bufferHistogram[MAX_FILE_PATH_CHARS] = "";
  double periodHigh, periodLow;
  double *pInputs = (double*)malloc((int)pParams->settings[CANVAS_SIZE_TAPUY]*(int)pParams->settings[CANVAS_SIZE_TAPUY] * sizeof(double));

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Tapuy run start");

  requestTempFileFolderPath(tempPath);

  sprintf(instanceIDName, "%d", (int)pParams->settings[STRATEGY_INSTANCE_ID]  );
  strcat(buffer, tempPath);
  strcat(buffer, instanceIDName);
  strcat(buffer, extension);

  sprintf(instanceIDName,  "%d", (int)pParams->settings[STRATEGY_INSTANCE_ID]);
  strcat(bufferHistogram, tempPath);
  strcat(bufferHistogram, instanceIDName);
  strcat(bufferHistogram, "_HIST.png");

  if ((int)pParams->settings[SCALING_MODE_TAPUY] == 1)
  findHighestAndLowest(pParams, &periodHigh, &periodLow);

	for(j = 0; j < (int)pParams->settings[INPUT_BARS_TAPUY] ; j++)
        {
          open[j]  = pParams->ratesBuffers->rates[1].open[pParams->ratesBuffers->rates[1].info.arraySize - j - 2];
		  close[j] = pParams->ratesBuffers->rates[1].close[pParams->ratesBuffers->rates[1].info.arraySize - j - 2];

		  if( (int)pParams->settings[USE_HIGH_LOW_TAPUY] == 1)
		  {
		    high[j]  = pParams->ratesBuffers->rates[1].high[pParams->ratesBuffers->rates[1].info.arraySize - j - 2];
		    low[j]   = pParams->ratesBuffers->rates[1].low[pParams->ratesBuffers->rates[1].info.arraySize - j - 2];
		  }  
        }

		drawChartForNN_tester(open, high, low, close, (int)pParams->settings[INPUT_BARS_TAPUY], buffer, (int)pParams->settings[CANVAS_SIZE_TAPUY], periodHigh, periodLow, (int)pParams->settings[SCALING_MODE_TAPUY], (int)pParams->settings[USE_HIGH_LOW_TAPUY]);

		ilGenImages( 1, &imgID );
        ilBindImage( imgID );

		File = fopen(buffer, "rb");
		fseek(File, 0, SEEK_END);
		Size = ftell(File);

		Lump = (ILubyte*)malloc(Size);
		fseek(File, 0, SEEK_SET);
		fread(Lump, 1, Size, File);
		fclose(File);

		ilLoadL(IL_PNG, Lump, Size);

		bytes = ilGetData();

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);	

		j = 0;


		for (h = 0; h < (int) height; h++)
		{
			for (w = 0; w < (int) width; w++)
			{
				pInputs[j] = bytes[(h*width + w)*4 + 0]; 
				j++ ;
			}
		}


		free(Lump);
		ilDeleteImages( 1, &imgID );

    /* Run the neural network */
	fann_run(pNeuralNetwork, pInputs);
    output = fann_run(pNeuralNetwork, pInputs);

	if ((int)pParams->settings[SAVE_HISTOGRAM_TAPUY] == 1)
	{
		drawHistogramForNN_tester(output, bufferHistogram);
	}


	predictionScore = 0;

		sumPositive = output[1] ;
		sumNegative = output[0] ;

	if (sumNegative > sumPositive )
    predictionScore = -1;

	if (sumPositive > sumNegative )
    predictionScore = 1;


    /* Destroy the neural network */
    fann_destroy(pNeuralNetwork);

	free(open);
	free(high);
	free(low);
	free(close);
	free(pInputs);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Tapuy run end");

  return predictionScore;
}

static NNCommitteeResult getPredictionTapuy(StrategyParams* pParams)
{
  NNCommitteeResult prediction = PREDICTION_NONE;
  double                predictionScore = 0;
  struct fann*       neuralNetwork ;
  unsigned int layers[NN_TOTAL_LAYERS_TAPUY];
   
  layers[0] = (int)pParams->settings[CANVAS_SIZE_TAPUY]*(int)pParams->settings[CANVAS_SIZE_TAPUY];
  layers[1] = (int)pParams->settings[CANVAS_SIZE_TAPUY]*(int)pParams->settings[CANVAS_SIZE_TAPUY]/5;
  layers[2] = NN_OUTPUTS_TAPUY;
    
  neuralNetwork = fann_create_standard_array(NN_TOTAL_LAYERS_TAPUY, layers);
   
  /* init prediction in case something goes wrong */
  prediction = PREDICTION_DISAGREEMENT;

  trainNeuralNetworksTapuy(pParams, neuralNetwork);

  predictionScore = runNeuralNetworksTapuy(pParams, neuralNetwork);

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"predictions score : %lf", predictionScore);
  
  
  if(predictionScore > 0)
  {
    prediction = PREDICTION_ALL_UP_MOVE;
  }
  else if(predictionScore < 0)
  {
    prediction = PREDICTION_ALL_DOWN_MOVE;
  }
  else
  {
    prediction = PREDICTION_DISAGREEMENT;
  }


  return prediction;
}


static void trainNeuralNetworksPaqarin(StrategyParams* pParams, struct fann** ppNeuralNetworks, double* pInputs)
{
 
  int i, k, m, n;
  double  output[4];
  double  body;
  int outBegIdx, outNBElement;
  double atr;
  double stopLossUsed   ;

  	

  for(i = 0; i < pParams->settings[COMMITTEE_SIZE_PAQARIN]; i++)
  {

    unsigned int layers[NN_TOTAL_LAYERS_PAQARIN];
    layers[0] = (unsigned int)pParams->settings[INPUT_BARS_PAQARIN];
    layers[1] = (unsigned int)pParams->settings[INPUT_BARS_PAQARIN]*4;
    layers[2] = NN_OUTPUTS_PAQARIN;
    ppNeuralNetworks[i] = fann_create_standard_array(NN_TOTAL_LAYERS_PAQARIN, layers);
    fann_randomize_weights(ppNeuralNetworks[i], MIN_WEIGHT, MAX_WEIGHT);
    fann_set_learning_rate(ppNeuralNetworks[i], NN_LEARNING_RATE);
    fann_set_learning_momentum(ppNeuralNetworks[i], NN_LEARNING_MOMENTUM);
    fann_set_activation_steepness_hidden(ppNeuralNetworks[i], STEEPNESS_HIDDEN);
    fann_set_activation_steepness_output(ppNeuralNetworks[i], STEEPNESS_OUTPUT);
    fann_set_activation_function_hidden(ppNeuralNetworks[i], FANN_SIGMOID);
    fann_set_activation_function_output(ppNeuralNetworks[i], FANN_SIGMOID);
    fann_set_training_algorithm(ppNeuralNetworks[i], FANN_TRAIN_RPROP);
    fann_reset_MSE(ppNeuralNetworks[i]);

    for(k = 0; k < pParams->settings[TRAINING_EPOCHS_COUNT_PAQARIN]; k++)
    {

		for(m = pParams->ratesBuffers->rates[1].info.arraySize - (int)pParams->settings[TRAINING_SETS_PER_EPOCH_PAQARIN]; m < pParams->ratesBuffers->rates[1].info.arraySize-1; m++)
      {
   
        for(n = 0; n < (int)pParams->settings[INPUT_BARS_PAQARIN]; n++)
        {
          pInputs[n] = (log((pParams->ratesBuffers->rates[1].open[m - n - 1])/(pParams->ratesBuffers->rates[1].open[m - n - 2])));
        }

		output[0] = 0;
		output[1] = 0;
		output[2] = 0;
		output[3] = 0;

		TA_ATR(m-1, m-1, pParams->ratesBuffers->rates[1].high, pParams->ratesBuffers->rates[1].low, pParams->ratesBuffers->rates[1].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);

		body = pParams->ratesBuffers->rates[1].open[m]-pParams->ratesBuffers->rates[1].open[m-1] ;

		stopLossUsed = atr * pParams->settings[SL_ATR_MULTIPLIER];

		if (body < 0)
          output[0] = 1;

		if (body > 0)
		  output[1] = 1;

		if (pParams->ratesBuffers->rates[1].open[m-1] - min(pParams->ratesBuffers->rates[1].open[m], pParams->ratesBuffers->rates[1].low[m-1]) > stopLossUsed)
          output[2] = 1;

		if (max(pParams->ratesBuffers->rates[1].open[m], pParams->ratesBuffers->rates[1].high[m-1]) - pParams->ratesBuffers->rates[1].open[m-1] > stopLossUsed)
		  output[3] = 1;

        /* Train the neural network */
        fann_train(ppNeuralNetworks[i], pInputs, output);
      }

    }
  }  
}

static int runNeuralNetworksPaqarin(StrategyParams* pParams, struct fann** ppNeuralNetworks, double* pInputs)
{

  int i, j, predictionScore = 0;
  double *output;


  output = 0;

  for(i = 0; i < pParams->settings[COMMITTEE_SIZE_PAQARIN]; i++)
  {

    for(j = 0; j < (int)pParams->settings[INPUT_BARS_PAQARIN]; j++)
    {
		pInputs[j] = log((pParams->ratesBuffers->rates[1].open[pParams->ratesBuffers->rates[1].info.arraySize - j - 1])/(pParams->ratesBuffers->rates[1].open[pParams->ratesBuffers->rates[1].info.arraySize - j - 2]));
    }

    /* Run the neural network */
    output = fann_run(ppNeuralNetworks[i], pInputs);

	if (output != output)
	output = 0;

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)",%lf,%lf,%lf,%lf",output[0], output[1], output[2], output[3]);

    if(output[0] > output[1] && output[3] < 0.2)
    {
      predictionScore--;
    }
    else if(output[1] > output[0] && output[2] < 0.2)
    {
      predictionScore++;
    }

    /* Destroy the neural network */
    fann_destroy(ppNeuralNetworks[i]);
  }

  return predictionScore;
}

static NNCommitteeResult getPredictionPaqarin(StrategyParams* pParams)
{
  NNCommitteeResult prediction = PREDICTION_NONE;
  double*           inputs = (double*)malloc((int)pParams->settings[INPUT_BARS_PAQARIN] * sizeof(double)), unroundedScoreThreshold = 0.01 * 100 * pParams->settings[COMMITTEE_SIZE_PAQARIN];
  int                predictionScore = 0, scoreThreshold = (int)((unroundedScoreThreshold > (floor(unroundedScoreThreshold)+0.5f)) ? ceil(unroundedScoreThreshold) : floor(unroundedScoreThreshold));
  struct fann**     neuralNetworks = (struct fann**)malloc((int)pParams->settings[COMMITTEE_SIZE_PAQARIN] * sizeof(struct fann*));
   
  /* init prediction in case something goes wrong */
  prediction = PREDICTION_DISAGREEMENT;

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Paqarin training start");

  trainNeuralNetworksPaqarin(pParams, neuralNetworks, inputs);

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Paqarin training start");

  predictionScore = runNeuralNetworksPaqarin(pParams, neuralNetworks, inputs);

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Paqarin prediction end");
  
  if(predictionScore >= scoreThreshold)
  {
    prediction = PREDICTION_ALL_UP_MOVE;
  }
  else if(predictionScore <= -scoreThreshold)
  {
    prediction = PREDICTION_ALL_DOWN_MOVE;
  }
  else
  {
    prediction = PREDICTION_DISAGREEMENT;
  }

  free(neuralNetworks);
  free(inputs);

  return prediction;
}

AsirikuyReturnCode runAsirikuyBrain(StrategyParams* pParams)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  NNCommitteeResult  predictionPaqarin, predictionSunqu, predictionTapuy;
  TA_RetCode         retCode;
  char               timeString[MAX_TIME_STRING_SIZE] = "";
  OrderType          openOrderType;
  double             atr, stopLoss, takeProfit;
  int                totalActiveOrders = 0, tradingSignals = 0, outBegIdx, outNBElement;
  int                shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[0].info.arraySize - 2;
  int				 shiftDaily1Index = pParams->ratesBuffers->rates[1].info.arraySize - 2;
   // custom ui variable definitions
  char *userInterfaceVariableNames[TOTAL_UI_VALUES] = {
                                           "ATR ",
										   "Prediction Paqarin", 
										   "Prediction Sunqu ",
										   "Prediction Tapuy ",
										   " ",
										   " ",
										   " ",
										   " ",
										   " ",
										   " "
											};

  double userInterfaceValues[TOTAL_UI_VALUES];

  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Hour is %d", hour());

  if(pParams == NULL)
  {
    pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runAsirikuyBrain() failed. pParams = NULL");
    return NULL_POINTER;
  }

  /* Load indicators */
  retCode = TA_ATR(shiftDaily1Index, shiftDaily1Index, pParams->ratesBuffers->rates[1].high, pParams->ratesBuffers->rates[1].low, pParams->ratesBuffers->rates[1].close, (int)pParams->settings[ATR_AVERAGING_PERIOD], &outBegIdx, &outNBElement, &atr);
  if(retCode != TA_SUCCESS)
  {
    return logTALibError("TA_ATR()", retCode);
  }

  if(totalOpenOrders(pParams, BUY) > 0 )
  {
    openOrderType = BUY;
  }
  else if(totalOpenOrders(pParams, SELL) > 0 )
  {
    openOrderType = SELL;
  }
  else
  {
    openOrderType = NONE;
  }

  

  if(((parameter(TRADE_ON_HOUR) == -1) && (hour() == 0 || (hour()==4 && dayOfWeek()==1))) ||
	  (parameter(TRADE_ON_HOUR) == hour())){

  if (parameter(USE_PAQARIN) == TRUE)
  predictionPaqarin = getPredictionPaqarin(pParams);

  if (parameter(USE_SUNQU) == TRUE)
  predictionSunqu = getPredictionSunqu(pParams);

  if (parameter(USE_TAPUY) == TRUE)
  predictionTapuy = getPredictionTapuy(pParams);

   // set ui values
	userInterfaceValues[0] = atr ;
	userInterfaceValues[1] = predictionPaqarin ;
	userInterfaceValues[2] = predictionSunqu ;
	userInterfaceValues[3] = predictionTapuy ;
	userInterfaceValues[4] = 0 ;
    userInterfaceValues[5] = 0 ;
	userInterfaceValues[6] = 0 ;
	userInterfaceValues[7] = 0 ;
	userInterfaceValues[8] = 0 ;
	userInterfaceValues[9] = 0 ;

 // call ui file saving
	saveUserInterfaceValues(userInterfaceVariableNames, userInterfaceValues, 10,  (int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

	if (parameter(USE_PAQARIN) == FALSE)
	{
		if (parameter(USE_SUNQU) == TRUE)
		predictionPaqarin = predictionSunqu;

		if (parameter(USE_TAPUY) == TRUE)
		predictionPaqarin = predictionTapuy;
	}

	if (parameter(USE_SUNQU) == FALSE)
	{
		if (parameter(USE_PAQARIN) == TRUE)
		predictionSunqu = predictionPaqarin;

		if (parameter(USE_TAPUY) == TRUE)
		predictionSunqu = predictionTapuy;
	}

	if (parameter(USE_TAPUY) == FALSE)
	{
		if (parameter(USE_SUNQU) == TRUE)
		predictionTapuy = predictionSunqu;

		if (parameter(USE_PAQARIN) == TRUE)
		predictionTapuy = predictionPaqarin;
	}

 
	
  safe_timeString(timeString, pParams->ratesBuffers->rates[0].time[shift0Index]);
  pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"GlobalNN InstanceID = %d, BarTime = %s, ATR = %lf, Paqarin = %d, Sunqu = %d, Tapuy = %d", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, atr, (int)predictionPaqarin, (int)predictionSunqu, (int)predictionTapuy);
  }

  stopLoss   = atr * pParams->settings[SL_ATR_MULTIPLIER];
  takeProfit = atr * pParams->settings[TP_ATR_MULTIPLIER];
  
  /* if orders were not set properly (SL or TP = 0) then set stops so
     that the front-end can properly set these values.            */ 
  if(!areOrdersCorrect(pParams, pParams->settings[USE_SL], pParams->settings[USE_TP]))
  {
    setStops(pParams, 0, 0, stopLoss, 0, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  if(((parameter(TRADE_ON_HOUR) == -1) && (hour() == 0 || (hour()==4 && dayOfWeek()==1))) ||
	  (parameter(TRADE_ON_HOUR) == hour())){

  if((predictionPaqarin == PREDICTION_ALL_UP_MOVE) && (predictionSunqu == PREDICTION_ALL_UP_MOVE) && (predictionTapuy == PREDICTION_ALL_UP_MOVE))
  {
    return openOrUpdateLongTrade(pParams, 0, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }
  else if((predictionPaqarin == PREDICTION_ALL_DOWN_MOVE) && (predictionSunqu == PREDICTION_ALL_DOWN_MOVE) && (predictionTapuy == PREDICTION_ALL_DOWN_MOVE)) 
  {
    return openOrUpdateShortTrade(pParams, 0, 0, stopLoss, takeProfit,1, USE_INTERNAL_SL, USE_INTERNAL_TP);
  }

  }
  

  /*
  if((openOrderType == BUY) && 
	  ((predictionPaqarin == PREDICTION_ALL_DOWN_MOVE || predictionPaqarin == PREDICTION_DISAGREEMENT) ||
	   (predictionSunqu == PREDICTION_ALL_DOWN_MOVE || predictionSunqu == PREDICTION_DISAGREEMENT) ||
	   (predictionTapuy == PREDICTION_ALL_DOWN_MOVE || predictionTapuy == PREDICTION_DISAGREEMENT))
	  )
  {
    returnCode = closeLongTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runPaqarin()", returnCode);
    }
  }
  else if((openOrderType == SELL) && 
	  ((predictionPaqarin == PREDICTION_ALL_UP_MOVE || predictionPaqarin == PREDICTION_DISAGREEMENT) ||
	   (predictionSunqu == PREDICTION_ALL_UP_MOVE || predictionSunqu == PREDICTION_DISAGREEMENT) ||
	   (predictionTapuy == PREDICTION_ALL_UP_MOVE || predictionTapuy == PREDICTION_DISAGREEMENT))
	   )
  {
    returnCode = closeShortTrade(pParams, 0);
    if(returnCode != SUCCESS)
    {
      return logAsirikuyError("runPaqarin()", returnCode);
    }
  }*/

  if ((pParams->bidAsk.bid[0]+stopLoss < pParams->orderInfo[0].stopLoss) &&
		(totalOpenOrders(pParams, SELL) > 0)){
		modifyTradeEasy(SELL , -1, stopLoss, 0);
	}

	if ((pParams->bidAsk.ask[0]-stopLoss > pParams->orderInfo[0].stopLoss) &&
		(totalOpenOrders(pParams, BUY) > 0) ){
	    modifyTradeEasy(BUY , -1, stopLoss, 0);
	}

  return SUCCESS;
}