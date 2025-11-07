/**
 * @file
 * @brief     Machine learning functions to be called from C functions.
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
#include "AsirikuyTime.h"
#include "EasyTradeCWrapper.hpp"
#include "AsirikuyMachineLearningCWrapper.hpp"
#include "InputOutputGeneratorLibrary.hpp"

// shark general includes
#include<shark/Data/Dataset.h>
#include <shark/Data/DataDistribution.h>

// SVM related
#include <shark/Algorithms/Trainers/CSvmTrainer.h>
#include <shark/Models/Kernels/GaussianRbfKernel.h>
#include <shark/Models/Kernels/LinearKernel.h>
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h>
#include <shark/Models/Converter.h>
#include <shark/Models/ConcatenatedModel.h>

// Linear classifier includes
#include <shark/Algorithms/Trainers/LDA.h>

// Linear regression
#include <shark/Algorithms/Trainers/LinearRegression.h>
#include <shark/ObjectiveFunctions/Loss/SquaredLoss.h>
#include <shark/ObjectiveFunctions/Loss/TukeyBiweightLoss.h>


// RBM related includes
#include <shark/Unsupervised/RBM/BinaryRBM.h>
#include <shark/Unsupervised/RBM/analytics.h>
#include <shark/Algorithms/GradientDescent/SteepestDescent.h>
#include <shark/Rng/GlobalRng.h>


// nearest neighbour
#include <shark/Models/NearestNeighborClassifier.h>//the classifier
#include <shark/Algorithms/NearestNeighbors/TreeNearestNeighbors.h>//nearest neeighbor search using trees
#include <shark/Models/Trees/KDTree.h>//KD tree for nearest neighbor lookup

//random forrest
#include <shark/Algorithms/Trainers/RFTrainer.h>

// Neural network
#include <shark/Algorithms/GradientDescent/Rprop.h>
#include <shark/ObjectiveFunctions/ErrorFunction.h>
#include <shark/ObjectiveFunctions/Loss/SquaredLoss.h>
#include <shark/Models/Autoencoder.h>// the autoencoder to train unsupervised
#include <shark/Models/ImpulseNoiseModel.h>// model adding noise to the inputs
#include <shark/Models/ConcatenatedModel.h>// to concatenate Autoencoder with noise adding model
#include <shark/Models/FFNet.h>
#include <shark/ObjectiveFunctions/Regularizer.h> //L1 and L2 regularisation
#include <shark/Algorithms/GradientDescent/SteepestDescent.h> //optimizer: simple gradient descent.
#include <shark/ObjectiveFunctions/Loss/CrossEntropy.h> // loss used for supervised training
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h> // loss used for evaluation of performance

#include "linreg.h"
#include <stdint.h>

using namespace shark;
using namespace std;

typedef enum outcomes_t
{
	OUTCOME_NO_ENTRY    = 0,
	OUTCOME_LONG_ENTRY  = 1,
	OUTCOME_SHORT_ENTRY = 2

} outcomes;

double performLinearRegression(double* x, double* y, int dataSize)
{
	LinearRegression1 lr(x, y, dataSize);

	return lr.getCoefDeterm();
}

uint8_t KNN_Prediction_i_simpleReturn_o_mlemse(int learningPeriod, int barsUsed, int frontier)
{
	const unsigned int K = 10;

	ClassificationDataset dataset  = classification_i_simpleReturn_o_mlemse(learningPeriod, barsUsed, frontier);

	KDTree<RealVector> tree(dataset.inputs()); 
	TreeNearestNeighbors<RealVector,unsigned int> algorithm(dataset,&tree);
	NearestNeighborClassifier<RealVector> KNN(&algorithm,K);

	Data<unsigned int> testOutput;
	ClassificationDataset datasetInput = p_classification_i_simpleReturn(barsUsed);
	testOutput = KNN(datasetInput.inputs());

	if (testOutput.element(0) == 1) return(2);
	if (testOutput.element(0) == 0) return(1);

	return(0);
}

uint8_t LR_Prediction_i_simpleReturn_o_mlemse(int learningPeriod, int barsUsed, int frontier)
{      

   RegressionDataset dataset  = regression_i_simpleReturn_o_mlemse(learningPeriod, barsUsed, frontier);
   LinearModel<> model;
   LinearRegression trainer;
   trainer.train(model, dataset);
   
    
   Data<RealVector> testOutput;
   RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
   testOutput = model(datasetInput.inputs());
   
    char filenameString[MAX_TIME_STRING_SIZE] = "";
    
    if (testOutput.element(0)[0] > testOutput.element(0)[1]) return(1);
    if (testOutput.element(0)[1] > testOutput.element(0)[0]) return(2);
    
   return(0);
}

uint8_t NN_Prediction_i_simpleReturn_o_mlemse(int learningPeriod, int barsUsed, int frontier)
{
	//get problem data
	RegressionDataset dataset = regression_i_simpleReturn_o_mlemse(learningPeriod, barsUsed, frontier);
   
	FFNet<FastSigmoidNeuron,FastSigmoidNeuron> network;

	unsigned numInput=barsUsed;
	unsigned numHidden=2;
	unsigned numOutput=2;
	unsigned numberOfSteps=25*barsUsed;
	unsigned step;

	network.setStructure(numInput, numHidden, numOutput);
	initRandomNormal(network,1.5);

	SquaredLoss<> loss;
    ErrorFunction error(dataset, &network,&loss);

	RpropMinus optimizer;
	optimizer.init(error);
	for(step = 0; step < numberOfSteps; ++step){
      optimizer.step(error);
	}

	network.setParameterVector(optimizer.solution().point); // set weights to weights found by learning

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = network(datasetInput.inputs());

	if (testOutput.element(0)[0] > testOutput.element(0)[1]) return(1);
    if (testOutput.element(0)[1] > testOutput.element(0)[0]) return(2);

	return(0);
}

void LR_Prediction_i_simpleReturn_o_tradeOutcomeSLTP(int learningPeriod, int barsUsed, int tradingHour, double SL, double TP, int frontier, int *results)
{

   RegressionDataset dataset  = regression_i_simpleReturn_o_tradeOutcomeSLTP(learningPeriod, barsUsed, tradingHour, SL, TP, frontier);
   LinearModel<> model;
   LinearRegression trainer;
   trainer.train(model, dataset);
    
   Data<RealVector> testOutput;
   RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
   testOutput = model(datasetInput.inputs());
   
   results[0] = 0;
   results[1] = 0;
    
   if (testOutput.element(0)[0] > 0)  results[0] = 1;
   if (testOutput.element(0)[1] > 0) results[1] = 1;
        
   return;
}

double KNN_Prediction_i_returnBinary_o_mfemae(int learningPeriod, int barsUsed, int tradingHour, int barThreshold)
{
	const unsigned int K = 10;

	//get problem data
	ClassificationDataset dataset  = classification_i_simpleReturn_o_mfemaeComparison(learningPeriod, barsUsed, tradingHour, barThreshold);
	KDTree<RealVector> tree(dataset.inputs()); 
	TreeNearestNeighbors<RealVector,unsigned int> algorithm(dataset,&tree);
	NearestNeighborClassifier<RealVector> KNN(&algorithm,K);
    
	Data<unsigned int> testOutput;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutput = KNN(datasetInput.inputs());

	if (testOutput.element(0) == 1 ) return(1);
	if (testOutput.element(0) == 0 ) return(-1);

	return(0);
}

double KNN_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed)
{
	const unsigned int K = 100;

	//get problem data
	ClassificationDataset dataset  = classification_i_returnBinary_o_returnBinary(learningPeriod, barsUsed);

	KDTree<RealVector> tree(dataset.inputs()); 
	TreeNearestNeighbors<RealVector,unsigned int> algorithm(dataset,&tree);
	NearestNeighborClassifier<RealVector> KNN(&algorithm,K);

	Data<unsigned int> testOutput;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutput = KNN(datasetInput.inputs());

	if (testOutput.element(0) == 1) return(1);
	if (testOutput.element(0) == 0) return(-1);

	return(0);
}

double SVM_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed)
{
	/* definition of the Complexity parameter used for SVM creation */
	bool bias = true;           // use bias/offset parameter
	double C=10;
	double gamma = 0.001;

	//get problem data
	ClassificationDataset dataset = classification_i_returnBinary_o_returnBinary(learningPeriod, barsUsed);

	GaussianRbfKernel<> kernel(gamma); 
    KernelClassifier<RealVector> ke; 
	CSvmTrainer<RealVector> trainer(&kernel, C, bias);
	trainer.train(ke, dataset);

	Data<unsigned int> testOutput;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutput = ke(datasetInput.inputs());

	if (testOutput.element(0) == 1) return(1);
	if (testOutput.element(0) == 0) return(-1);

	return(0);
}

double LM_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed)
{
	//get problem data
	ClassificationDataset dataset  = classification_i_returnBinary_o_returnBinary(learningPeriod, barsUsed);

	LDA ldaTrainer;
	LinearClassifier<> lda;

	ldaTrainer.train(lda, dataset);

	Data<unsigned int> output;  // real-valued output of the machine
	output = lda(dataset.inputs()); // evaluate on training set

	Data<unsigned int> testOutput;

	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);

	testOutput = lda(datasetInput.inputs());

	if (testOutput.element(0) == 1) return(1);
	if (testOutput.element(0) == 0) return(-1);

	return(0);
}

double LR_Prediction_i_simpleReturn_o_tradeOutcome(int learningPeriod, int barsUsed, int tradingHour, int BE, double initial_SL, int frontier, double minStop, int dsl_type)
{

   RegressionDataset dataset  = regression_i_simpleReturn_o_tradeOutcome(learningPeriod, barsUsed, tradingHour, BE, initial_SL, frontier, minStop, dsl_type);
   LinearModel<> model;
   LinearRegression trainer;
   trainer.train(model, dataset);
    
   Data<RealVector> testOutput;
   RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
   testOutput = model(datasetInput.inputs());
    
   if (testOutput.element(0)[0] > 0 && testOutput.element(0)[1] < 0) return(1);
    if (testOutput.element(0)[1] > 0 && testOutput.element(0)[0] < 0) return(-1);
        
   return(0);
}

double NN_Prediction_i_simpleReturn_o_tradeOutcome(int learningPeriod, int barsUsed, int tradingHour, int BE, double initial_SL, int frontier, double minStop, int dsl_type, int trainingEpochs)
{
	//get problem data
	RegressionDataset dataset = regression_i_simpleReturn_o_tradeOutcome(learningPeriod, barsUsed, tradingHour, BE, initial_SL, frontier, minStop, dsl_type);
   
	FFNet<FastSigmoidNeuron,FastSigmoidNeuron> network;

	unsigned numInput=barsUsed;
	unsigned numHidden=(int)((barsUsed+2)/2);
	unsigned numOutput=2;
	unsigned numberOfSteps=trainingEpochs;
	unsigned step;

	network.setStructure(numInput, numHidden, numOutput);
	initRandomUniform(network,-0.1,0.1);

	SquaredLoss<> loss;
    ErrorFunction error(dataset, &network,&loss);

	RpropMinus optimizer;
	optimizer.init(error);
	for(step = 0; step < numberOfSteps; ++step){
      optimizer.step(error);
	}

	network.setParameterVector(optimizer.solution().point); // set weights to weights found by learning

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = network(datasetInput.inputs());

	if (testOutput.element(0)[0] > 0 && testOutput.element(0)[1] < 0) return(1);
    if (testOutput.element(0)[1] > 0 && testOutput.element(0)[0] < 0) return(-1);

	return(0);
}




double LM_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, int hourFilter, int barThreshold)
{
	//get problem data
	ClassificationDataset datasetLong = classification_i_returnBinary_o_tradeSuccess(learningPeriod,barsUsed, BUY, hourFilter, barThreshold);
	ClassificationDataset datasetShort = classification_i_returnBinary_o_tradeSuccess(learningPeriod,barsUsed, SELL, hourFilter, barThreshold);
	/* definition of the dataset that will be populated with examples */
	LDA ldaTrainerLong;
	LinearClassifier<> ldaLong;

	ldaTrainerLong.train(ldaLong, datasetLong);

	pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"3"); 

	Data<unsigned int> outputLong;  // real-valued output of the machine
	outputLong = ldaLong(datasetLong.inputs()); // evaluate on training set

	LDA ldaTrainerShort;
	LinearClassifier<> ldaShort;

	ldaTrainerLong.train(ldaShort, datasetShort);

	Data<unsigned int> outputShort;  // real-valued output of the machine
	outputShort = ldaShort(datasetShort.inputs()); // evaluate on training set

	Data<unsigned int> testOutputLong;
	Data<unsigned int> testOutputShort;

	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);

	testOutputLong = ldaLong(datasetInput.inputs());
	testOutputShort = ldaShort(datasetInput.inputs());

	if (testOutputLong.element(0) == 1 && testOutputShort.element(0) == 0) return(1);
	if (testOutputLong.element(0) == 0 && testOutputShort.element(0) == 1) return(-1);

	return(0);
}


double LR_Prediction_i_simpleReturn_o_simpleReturn(int learningPeriod, int barsUsed)
{

	//get problem data
	RegressionDataset dataset = regression_i_simpleReturn_o_simpleReturn(learningPeriod,barsUsed);

	/* definition of the dataset that will be populated with examples */
	LinearModel<> model;
	LinearRegression trainer;
	trainer.train(model, dataset);

	Data<RealVector> output;  // real-valued output of the machine
	output = model(dataset.inputs()); // evaluate on training set

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = model(datasetInput.inputs());

	return(testOutput.element(0)[0]);
}

double SVM_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, double C, double gamma, int tradingHour, int barThreshold)
{
	/* definition of the Complexity parameter used for SVM creation */
	bool bias = true;           // use bias/offset parameter

	//get problem data
	ClassificationDataset datasetLong  = classification_i_returnBinary_o_tradeSuccess(learningPeriod, barsUsed, BUY, tradingHour, barThreshold);
	ClassificationDataset datasetShort  = classification_i_returnBinary_o_tradeSuccess(learningPeriod, barsUsed, SELL, tradingHour, barThreshold);

	GaussianRbfKernel<> kernelLong(gamma); 
    KernelClassifier<RealVector> keLong; 
	CSvmTrainer<RealVector> trainerLong(&kernelLong, C, bias);
	trainerLong.train(keLong, datasetLong);

	GaussianRbfKernel<> kernelShort(gamma); 
    KernelClassifier<RealVector> keShort; 
	CSvmTrainer<RealVector> trainerShort(&kernelShort, C, bias);
	trainerShort.train(keShort, datasetShort);

	Data<unsigned int> testOutputLong;
	Data<unsigned int> testOutputShort;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutputLong = keLong(datasetInput.inputs());
	testOutputShort = keShort(datasetInput.inputs());

	if (testOutputLong.element(0) == 1 && testOutputShort.element(0) == 0) return(1);
	if (testOutputLong.element(0) == 0 && testOutputShort.element(0) == 1) return(-1);

	return(0);
}

double SVM_Prediction_i_returnBinary_o_returnBinary_direction(int learningPeriod, int barsUsed)
{
	/* definition of the Complexity parameter used for SVM creation */
	bool bias = true;           // use bias/offset parameter
	double C = 10;
	double gamma = 0.001;
	//get problem data
	ClassificationDataset datasetLong  = classification_i_returnBinary_o_returnBinary_direction(learningPeriod, barsUsed, BUY);
	ClassificationDataset datasetShort  = classification_i_returnBinary_o_returnBinary_direction(learningPeriod, barsUsed, SELL);

	GaussianRbfKernel<> kernelLong(gamma); 
    KernelClassifier<RealVector> keLong; 
	CSvmTrainer<RealVector> trainerLong(&kernelLong, C, bias);
	trainerLong.train(keLong, datasetLong);

	GaussianRbfKernel<> kernelShort(gamma); 
    KernelClassifier<RealVector> keShort; 
	CSvmTrainer<RealVector> trainerShort(&kernelShort, C, bias);
	trainerShort.train(keShort, datasetShort);

	Data<unsigned int> testOutputLong;
	Data<unsigned int> testOutputShort;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutputLong = keLong(datasetInput.inputs());
	testOutputShort = keShort(datasetInput.inputs());

	if (testOutputLong.element(0) == 1 && testOutputShort.element(0) == 0) return(1);
	if (testOutputLong.element(0) == 0 && testOutputShort.element(0) == 1) return(-1);

	return(0);
}

double KNN_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, int tradingHour, int barThreshold)
{
	const unsigned int K = 50;

	//get problem data
	ClassificationDataset datasetLong  = classification_i_returnBinary_o_tradeSuccess(learningPeriod, barsUsed, BUY, tradingHour, barThreshold);
	ClassificationDataset datasetShort  = classification_i_returnBinary_o_tradeSuccess(learningPeriod, barsUsed, SELL, tradingHour, barThreshold);

	KDTree<RealVector> treeLong(datasetLong.inputs()); 
	TreeNearestNeighbors<RealVector,unsigned int> algorithmLong(datasetLong,&treeLong);
	NearestNeighborClassifier<RealVector> KNNLong(&algorithmLong,K);

	KDTree<RealVector> treeShort(datasetShort.inputs()); 
	TreeNearestNeighbors<RealVector,unsigned int> algorithmShort(datasetShort,&treeShort);
	NearestNeighborClassifier<RealVector> KNNShort(&algorithmShort,K);
    
	Data<unsigned int> testOutputLong;
	Data<unsigned int> testOutputShort;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutputLong = KNNLong(datasetInput.inputs());
	testOutputShort = KNNShort(datasetInput.inputs());

	if (testOutputLong.element(0) == 1 && testOutputShort.element(0) == 0) return(1);
	if (testOutputLong.element(0) == 0 && testOutputShort.element(0) == 1) return(-1);

	return(0);
}



double SVM_Prediction_i_returnBinary_o_tradeSuccess_daily(double atrThreshold, int atrPeriod, int learningPeriod, int barsUsed, int barThreshold)
{
	/* definition of the Complexity parameter used for SVM creation */
	bool bias = true;           // use bias/offset parameter
	double C=10;
	double gamma = 0.001;

	//get problem data
	ClassificationDataset datasetLong  = classification_i_returnBinary_o_tradeSuccess_daily(atrThreshold, atrPeriod, learningPeriod, barsUsed, barThreshold);

	GaussianRbfKernel<> kernelLong(gamma); 
    KernelClassifier<RealVector> keLong; 
	CSvmTrainer<RealVector> trainerLong(&kernelLong, C, bias);
	trainerLong.train(keLong, datasetLong);

	Data<unsigned int> testOutputLong;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutputLong = keLong(datasetInput.inputs());

	if (testOutputLong.element(0) == 1) return(1);
	if (testOutputLong.element(0) == 0) return(-1);

	return(0);
}

double KNN_Prediction_i_returnBinary_o_tradeSuccess_daily(double atrThreshold, int atrPeriod, int learningPeriod, int barsUsed, int barThreshold)
{
	const unsigned int K = 50;

	//get problem data
	ClassificationDataset datasetLong  = classification_i_returnBinary_o_tradeSuccess_daily(atrThreshold, atrPeriod, learningPeriod, barsUsed, barThreshold);

	KDTree<RealVector> treeLong(datasetLong.inputs()); 
	TreeNearestNeighbors<RealVector,unsigned int> algorithmLong(datasetLong,&treeLong);
	NearestNeighborClassifier<RealVector> KNNLong(&algorithmLong,K);
    
	Data<unsigned int> testOutputLong;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutputLong = KNNLong(datasetInput.inputs());

	if (testOutputLong.element(0) == 1 ) return(1);
	if (testOutputLong.element(0) == 0 ) return(-1);

	return(0);
}

double KNN_Prediction_i_returnBinary_o_tradeSuccess_daily_adaptive(int barThreshold)
{
	const unsigned int K = 50;
	int i, j, n;
	double totalProfit;
	double totalLoss;
	double maxProfitFactor = 0;
	double selectedLearning, selectedBars;

	for(i=0;i<10;i++){
		for(j=0;j<10;j++){

			totalProfit = 0;
			totalLoss = 0;

			for(n=1;n<200;n++){

				ClassificationDataset datasetLong  = classification_i_returnBinary_o_tradeSuccess_daily_shift(n, i*2*10+50, (j+1)*2, barThreshold);
				KDTree<RealVector> treeLong(datasetLong.inputs()); 
				TreeNearestNeighbors<RealVector,unsigned int> algorithmLong(datasetLong,&treeLong);
				NearestNeighborClassifier<RealVector> KNNLong(&algorithmLong,K);
    
				Data<unsigned int> testOutputLong;
				ClassificationDataset datasetInput = p_classification_i_returnBinary_shift(n, (j+1)*2);
				testOutputLong = KNNLong(datasetInput.inputs());

				if ( (testOutputLong.element(0) == 1 && (cOpen(n-1) > cOpen(n))) ||
					 (testOutputLong.element(0) == 0 && (cOpen(n-1) < cOpen(n)))){
					totalProfit += fabs(cOpen(n-1) - cOpen(n));
				}

				if ( (testOutputLong.element(0) == 1 && (cOpen(n-1) < cOpen(n))) ||
					 (testOutputLong.element(0) == 0 && (cOpen(n-1) > cOpen(n)))){
					totalLoss += fabs(cOpen(n-1) - cOpen(n));
				}
			}

			if(totalProfit/totalLoss > maxProfitFactor){
				maxProfitFactor = totalProfit/totalLoss;
				selectedLearning = i*2*10+50;
				selectedBars =(j+1)*2;
			}
		}	
	}

	pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)", %lf, %lf, %lf",selectedLearning, selectedBars, maxProfitFactor);

	ClassificationDataset datasetLong  = classification_i_returnBinary_o_tradeSuccess_daily_shift(0, selectedLearning, selectedBars, barThreshold);
	KDTree<RealVector> treeLong(datasetLong.inputs()); 
	TreeNearestNeighbors<RealVector,unsigned int> algorithmLong(datasetLong,&treeLong);
	NearestNeighborClassifier<RealVector> KNNLong(&algorithmLong,K);
    
	Data<unsigned int> testOutputLong;
	ClassificationDataset datasetInput = p_classification_i_returnBinary_shift(0, selectedBars);
	testOutputLong = KNNLong(datasetInput.inputs());

	if (testOutputLong.element(0) == 1 ) return(1);
	if (testOutputLong.element(0) == 0 ) return(-1);

	return(0);
}

double SVM_Prediction_i_returnBinary_o_returnBinary_adaptive()
{
	double C= 10;
	bool bias = true;
	double gamma = 0.001;
	int i, j, n;
	double totalProfit;
	double totalLoss;
	double maxProfitFactor = 0;
	double selectedLearning, selectedBars;
	int typeUsed = 0;

	for(i=0;i<10;i++){
		for(j=0;j<10;j++){

			totalProfit = 0;
			totalLoss = 0;

			for(n=1;n<20;n++){

				ClassificationDataset dataset = classification_i_returnBinary_o_returnBinary_shift(n, i*2*10+50, (j+1)*2);

				GaussianRbfKernel<> kernel(gamma); 
				KernelClassifier<RealVector> ke; 
				CSvmTrainer<RealVector> trainer(&kernel, C, bias);
				trainer.train(ke, dataset);

				Data<unsigned int> testOutput;
				ClassificationDataset datasetInput = p_classification_i_returnBinary_shift(n, (j+1)*2);
				testOutput = ke(datasetInput.inputs());

				if ( (testOutput.element(0) == 1 && (cOpen(n-1) > cOpen(n))) ||
					 (testOutput.element(0) == 0 && (cOpen(n-1) < cOpen(n)))){
					totalProfit += fabs(cOpen(n-1) - cOpen(n));
				}

				if ( (testOutput.element(0) == 1 && (cOpen(n-1) < cOpen(n))) ||
					 (testOutput.element(0) == 0 && (cOpen(n-1) > cOpen(n)))){
					totalLoss += fabs(cOpen(n-1) - cOpen(n));
				}
			}

			if (totalLoss == 0) totalLoss = 0.00001;

			if (totalLoss != 0){
				if(totalProfit/totalLoss > maxProfitFactor){
					maxProfitFactor = totalProfit/totalLoss;
					selectedLearning = i*2*10+50;
					selectedBars =(j+1)*2;
					typeUsed = 0;
				}
			}
		}	
	}

	pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)", %lf, %lf, %lf, %d",selectedLearning, selectedBars, maxProfitFactor, typeUsed);

	if (maxProfitFactor > 1)
		return(0);

	ClassificationDataset dataset = classification_i_returnBinary_o_returnBinary_shift(0, selectedLearning, selectedBars);

	GaussianRbfKernel<> kernel(gamma); 
	KernelClassifier<RealVector> ke; 
	CSvmTrainer<RealVector> trainer(&kernel, C, bias);
	trainer.train(ke, dataset);

	Data<unsigned int> testOutput;
	ClassificationDataset datasetInput = p_classification_i_returnBinary_shift(0, selectedBars);
	testOutput = ke(datasetInput.inputs());

	if (testOutput.element(0) == 1 ) return(1);
	if (testOutput.element(0) == 0 ) return(-1);

	return(0);
}

double LR_Prediction_i_simpleReturn_o_mfemaeDifference(int learningPeriod, int barsUsed, int tradingHour, int barThreshold)
{

    char timeString[MAX_TIME_STRING_SIZE] = "";
	RegressionDataset dataset  = regression_i_simpleReturn_o_mfemaeDifference(learningPeriod, barsUsed, tradingHour, barThreshold);
	LinearModel<> model;
	LinearRegression trainer;
	trainer.train(model, dataset);
    
	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = model(datasetInput.inputs());
    
    safe_timeString(timeString, openTime(0));
    pantheios_logprintf(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)", %s, %lf, %lf", timeString, testOutput.element(0)[0], testOutput.element(0)[1]);

	if (testOutput.element(0)[0] > 1.2*testOutput.element(0)[1]) return(1);
    if (testOutput.element(0)[1] > 1.2*testOutput.element(0)[0]) return(-1);

	return(0);
}

double RF_Prediction_i_simpleReturn_o_mfemaeDifference(int learningPeriod, int barsUsed, int tradingHour, int barThreshold)
{
	RegressionDataset dataset  = regression_i_simpleReturn_o_mfemaeDifference(learningPeriod, barsUsed, tradingHour, barThreshold);
	SquaredLoss<> loss;

	RFTrainer trainer;
    RFClassifier model;
	trainer.train(model, dataset);

	Data<RealVector> output;  
	output = model(dataset.inputs());
	double trainError ;
	trainError = loss.eval(dataset.labels(), output);

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = model(datasetInput.inputs());

	if (testOutput.element(0)[0] > 1.2*testOutput.element(0)[1]) return(1);
    if (testOutput.element(0)[1] > 1.2*testOutput.element(0)[0]) return(-1);

	return(0);
}

double LR_Prediction_i_simpleReturn_o_mfemaeDifference_daily(int learningPeriod, int barsUsed, int barThreshold)
{


	RegressionDataset dataset  = regression_i_simpleReturn_o_mfemaeDifference_daily(learningPeriod, barsUsed, barThreshold);

	LinearModel<> model;
	LinearRegression trainer;
	trainer.train(model, dataset);

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = model(datasetInput.inputs());

   //pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"Output prediction mle = %lf", testOutput.element(0)[0]);
   //pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"Output prediction mse = %lf", testOutput.element(0)[1]);

	if (testOutput.element(0)[0] > testOutput.element(0)[1]*1.2) return(1);
    if (testOutput.element(0)[1] > testOutput.element(0)[0]*1.2) return(-1);

	return(0);
}

double RF_Prediction_i_simpleReturn_o_mfemaeDifference_daily(int learningPeriod, int barsUsed, int barThreshold)
{
	//get problem data
	RegressionDataset dataset  = regression_i_simpleReturn_o_mfemaeDifference_daily(learningPeriod, barsUsed, barThreshold);

	ZeroOneLoss<unsigned int, RealVector> loss;

	RFTrainer trainer;
    RFClassifier model;

	trainer.train(model, dataset);

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = model(datasetInput.inputs());

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"Output prediction = %lf, ", testOutput.element(0)[0]);

	if (testOutput.element(0)[0] > testOutput.element(0)[1]*1.2) return(1);
    if (testOutput.element(0)[1] > testOutput.element(0)[0]*1.2) return(-1);
}

double RF_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed)
{
	//get problem data
	ClassificationDataset dataset  = classification_i_returnBinary_o_returnBinary(learningPeriod, barsUsed);

	ZeroOneLoss<unsigned int, RealVector> loss;

	RFTrainer trainer;
    RFClassifier model;
	trainer.train(model, dataset);

	Data<RealVector> testOutput;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutput = model(datasetInput.inputs());

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"Output prediction = %lf, ", testOutput.element(0)[0]);

	return(testOutput.element(0)[0]-0.5);
}

double RF_Volatility_Prediction(int learningPeriod, int barsUsed)
{
	//get problem data
	RegressionDataset dataset  = regression_i_range_o_range(learningPeriod, barsUsed);

	SquaredLoss<> loss;

	RFTrainer trainer;
    RFClassifier model;
	trainer.train(model, dataset);

	Data<RealVector> output;  
	output = model(dataset.inputs());
	double trainError ;
	trainError = loss.eval(dataset.labels(), output);

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_range(barsUsed);
	testOutput = model(datasetInput.inputs());

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Output prediction = %lf, ", testOutput.element(0)[0]);

	return(testOutput.element(0)[0]);

	return(0);
}

double NN_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, int tradingHour, int barThreshold)
{
	//get problem data
	ClassificationDataset datasetLong = classification_i_returnBinary_o_tradeSuccess(learningPeriod, barsUsed, BUY, tradingHour, barThreshold);
	ClassificationDataset datasetShort = classification_i_returnBinary_o_tradeSuccess(learningPeriod, barsUsed, SELL, tradingHour,barThreshold);

	FFNet<LogisticNeuron,LogisticNeuron> networkLong;
	FFNet<LogisticNeuron,LogisticNeuron> networkShort;

	unsigned numInput=barsUsed;
	unsigned numHidden=(int)(barsUsed);
	unsigned numOutput=1;
	unsigned numberOfSteps;
	unsigned step;

	networkLong.setStructure(numInput, numHidden, numOutput);
	initRandomUniform(networkLong,-0.1,0.1);
	networkShort.setStructure(numInput, numHidden, numOutput);
	initRandomUniform(networkShort,-0.1,0.1);
    
    CrossEntropy loss1Long;
    ErrorFunction errorLong(datasetLong, &networkLong,&loss1Long);

	IRpropPlus optimizerLong;
	optimizerLong.init(errorLong);
	numberOfSteps = 5000;
	for(step = 0; step < numberOfSteps; ++step){
      optimizerLong.step(errorLong);
	}

	networkLong.setParameterVector(optimizerLong.solution().point); // set weights to weights found by learning

	CrossEntropy loss1Short;
    ErrorFunction errorShort(datasetShort, &networkShort,&loss1Short);

	IRpropPlus optimizerShort;
	optimizerShort.init(errorShort);
	for(step = 0; step < numberOfSteps; ++step){
      optimizerShort.step(errorShort);
	}

	networkShort.setParameterVector(optimizerShort.solution().point); // set weights to weights found by learning

	Data<RealVector> testOutputLong;
	Data<RealVector> testOutputShort;
	ClassificationDataset datasetInput = p_classification_i_returnBinary(barsUsed);
	testOutputLong = networkLong(datasetInput.inputs());
	testOutputShort = networkShort(datasetInput.inputs());

	if (testOutputLong.element(0)[0] < 0.5 && testOutputShort.element(0)[0] > 0.5) return(1);
	if (testOutputLong.element(0)[0] > 0.5 && testOutputShort.element(0)[0] < 0.5) return(-1);

	return(0);
}

double NN_Prediction_i_simpleReturn_o_simpleReturn(int learningPeriod, int barsUsed, int training, int functionType)
{
	//get problem data
	RegressionDataset dataset = regression_i_simpleReturn_o_simpleReturn(learningPeriod, barsUsed);

	FFNet<FastSigmoidNeuron,LinearNeuron> network;

	/*if (functionType == 0)
	FFNet<FastSigmoidNeuron,LinearNeuron> network;

	if (functionType == 1)
	FFNet<TanhNeuron,TanhNeuron> network;*/

	unsigned numInput=barsUsed;
	unsigned numHidden=(int)(barsUsed);
	unsigned numOutput=1;
	unsigned numberOfSteps;
	unsigned step;

	network.setStructure(numInput, numHidden, numOutput);
	initRandomUniform(network,-1,1);

	SquaredLoss<> loss;
    ErrorFunction error(dataset, &network,&loss);

	IRpropPlus optimizer;
	optimizer.init(error);
	numberOfSteps = training;
	for(step = 0; step < numberOfSteps; ++step){
      optimizer.step(error);
	}

	network.setParameterVector(optimizer.solution().point); // set weights to weights found by learning

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = network(datasetInput.inputs());

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)", %lf", testOutput.element(0)[0] );


	if (testOutput.element(0)[0] > 0) return(1);
	if (testOutput.element(0)[0] < 0) return(-1);

	return(0);
}

double NN_Prediction_i_simpleReturn_o_mfemaeDifference_daily(int learningPeriod, int barsUsed, int training, int functionType)
{
	//get problem data
	RegressionDataset dataset = regression_i_simpleReturn_o_mfemaeDifference_daily(learningPeriod, barsUsed, 4);

	FFNet<FastSigmoidNeuron,FastSigmoidNeuron> network;

	unsigned numInput=barsUsed;
	unsigned numHidden=(int)(barsUsed*2);
	unsigned numOutput=2;
	unsigned numberOfSteps;
	unsigned step;

	network.setStructure(numInput, numHidden, numOutput);
	initRandomUniform(network,-1,1);

	SquaredLoss<> loss;
    ErrorFunction error(dataset, &network,&loss);

	IRpropPlus optimizer;
	optimizer.init(error);
	numberOfSteps = training;
	for(step = 0; step < numberOfSteps; ++step){
      optimizer.step(error);
	}

	network.setParameterVector(optimizer.solution().point); // set weights to weights found by learning

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = network(datasetInput.inputs());

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)", %lf", testOutput.element(0)[0] );


	if (testOutput.element(0)[0] > testOutput.element(0)[1]) return(1);
	if (testOutput.element(0)[0] < testOutput.element(0)[1]) return(-1);

	return(0);
}

double NN_Prediction_i_simpleReturn_o_mfemaeDifference(int learningPeriod, int barsUsed, int hourFilter, int training, int barThreshold)
{
	//get problem data
	RegressionDataset dataset = regression_i_simpleReturn_o_mfemaeDifference(learningPeriod, barsUsed, hourFilter, barThreshold);

	FFNet<FastSigmoidNeuron,FastSigmoidNeuron> network;

	unsigned numInput=barsUsed;
	unsigned numHidden=(int)(barsUsed*2);
	unsigned numOutput=2;
	unsigned numberOfSteps;
	unsigned step;

	network.setStructure(numInput, numHidden, numOutput);
	initRandomUniform(network,-1,1);

	SquaredLoss<> loss;
    ErrorFunction error(dataset, &network,&loss);

	IRpropPlus optimizer;
	optimizer.init(error);
	numberOfSteps = training;
	for(step = 0; step < numberOfSteps; ++step){
      optimizer.step(error);
	}

	network.setParameterVector(optimizer.solution().point); // set weights to weights found by learning

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = network(datasetInput.inputs());

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)", %lf", testOutput.element(0)[0] );


	if (testOutput.element(0)[0] > 1.2*testOutput.element(0)[1]) return(1);
	if (testOutput.element(0)[0] < 1.2*testOutput.element(0)[1]) return(-1);

	return(0);
}

// deep learning
typedef Autoencoder<RectifierNeuron,LinearNeuron> AutoencoderModel;//type of autoencoder
typedef FFNet<RectifierNeuron,LinearNeuron> Network;//final supervised trained structure

template<class AutoencoderModel>
AutoencoderModel trainAutoencoderModel(
	UnlabeledData<RealVector> const& data,//the data to train with
	std::size_t numHidden,//number of features in the AutoencoderModel
	double regularisation,//strength of the regularisation
	double noiseStrength, // strength of the added noise
	std::size_t iterations //number of iterations to optimize
){
//###end<function>
//###begin<model>	
	//create the model
	std::size_t inputs = dataDimension(data);
	AutoencoderModel baseModel;
	baseModel.setStructure(inputs, numHidden);
	initRandomUniform(baseModel,-0.1*std::sqrt(1.0/inputs),0.1*std::sqrt(1.0/inputs));
	ImpulseNoiseModel noise(noiseStrength,0.0);//set an input pixel with probability p to 0
	ConcatenatedModel<RealVector,RealVector> model = noise>> baseModel;
//###end<model>	
//###begin<objective>		
	//create the objective function
	LabeledData<RealVector,RealVector> trainSet(data,data);//labels identical to inputs
	SquaredLoss<RealVector> loss;
	ErrorFunction error(trainSet, &model, &loss);
	TwoNormRegularizer regularizer(error.numberOfVariables());
	error.setRegularizer(regularisation,&regularizer);
//###end<objective>	
	//set up optimizer
//###begin<optimizer>
	IRpropPlusFull optimizer;
	optimizer.init(error);
	for(std::size_t i = 0; i != iterations; ++i){
		optimizer.step(error);
		std::cout<<i<<" "<<optimizer.solution().value<<std::endl;
	}
//###end<optimizer>
	model.setParameterVector(optimizer.solution().point);
	return baseModel;
}

Network unsupervisedPreTraining(
        UnlabeledData<RealVector> const& data,
        std::size_t numHidden1,std::size_t numHidden2, std::size_t numOutputs,
        double regularisation, double noiseStrength, std::size_t iterations
){
        //train the first hidden layer
        AutoencoderModel layer =  trainAutoencoderModel<AutoencoderModel>(
                data,numHidden1,
                regularisation, noiseStrength,
                iterations
        );
        //compute the mapping onto the features of the first hidden layer
        UnlabeledData<RealVector> intermediateData = layer.evalLayer(0,data);

        //train the next layer
        AutoencoderModel layer2 =  trainAutoencoderModel<AutoencoderModel>(
                intermediateData,numHidden2,
                regularisation, noiseStrength,
                iterations
        );
        
        //create the final network
        Network network;
        network.setStructure(dataDimension(data),numHidden1,numHidden2, numOutputs);
        initRandomNormal(network,0.1);
        network.setLayer(0,layer.encoderMatrix(),layer.hiddenBias());
        network.setLayer(1,layer2.encoderMatrix(),layer2.hiddenBias());

        return network;
}

double DeepLearning_(int learningPeriod, int barsUsed, int training, int barThreshold)
{
    
    //model parameters
    std::size_t numHidden1 = 8;
    std::size_t numHidden2 = 8;
    //unsupervised hyper parameters
    double unsupRegularisation = 0.001;
    double noiseStrength = 0.3;
    std::size_t unsupIterations = 100;
    //supervised hyper parameters
    double regularisation = 0.0001;
    std::size_t iterations = 200;

	//get problem data
    
	ClassificationDataset data = classification_i_returnBinary_o_tradeSuccess_daily(1, 20, learningPeriod, barsUsed, barThreshold);

    //unsupervised pre training
    Network network = unsupervisedPreTraining(
        data.inputs(),numHidden1, numHidden2,numberOfClasses(data),
        unsupRegularisation, noiseStrength, unsupIterations
);


	//create the supervised problem. Cross Entropy loss with one norm regularisation
    CrossEntropy loss;
    ErrorFunction error(data, &network, &loss);
    OneNormRegularizer regularizer(error.numberOfVariables());
    error.setRegularizer(regularisation,&regularizer);
    
    //optimize the model
    IRpropPlusFull optimizer;
    optimizer.init(error);
    for(std::size_t i = 0; i != training; ++i){
        optimizer.step(error);
}
    network.setParameterVector(optimizer.solution().point);

	Data<RealVector> testOutput;
	RegressionDataset datasetInput = p_regression_i_simpleReturn(barsUsed);
	testOutput = network(datasetInput.inputs());

	pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)", %lf", testOutput.element(0)[0] );

	if (testOutput.element(0)[0] > 1.2*testOutput.element(0)[1]) return(1);
	if (testOutput.element(0)[0] < 1.2*testOutput.element(0)[1]) return(-1);

	return(0);
}