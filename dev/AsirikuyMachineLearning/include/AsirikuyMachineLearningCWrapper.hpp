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

#ifndef ASIRIKUY_MACHINE_LEARNING_C_WRAPPER_HPP_
#define ASIRIKUY_MACHINE_LEARNING_C_WRAPPER_HPP_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint8_t LR_Prediction_i_simpleReturn_o_mlemse(int learningPeriod, int barsUsed, int frontier);
uint8_t NN_Prediction_i_simpleReturn_o_mlemse(int learningPeriod, int barsUsed, int frontier);
uint8_t KNN_Prediction_i_simpleReturn_o_mlemse(int learningPeriod, int barsUsed, int frontier);

double KNN_Prediction_i_returnBinary_o_mfemae(int learningPeriod, int barsUsed, int tradingHour, int barThreshold);
double SVM_Prediction_i_returnBinary_o_returnBinary_direction(int learningPeriod, int barsUsed);
double RF_Prediction_i_simpleReturn_o_mfemaeDifference_daily(int learningPeriod, int barsUsed, int barThreshold);
double LR_Prediction_i_simpleReturn_o_mfemaeDifference_daily(int learningPeriod, int barsUsed, int barThreshold);
double KNN_Prediction_i_returnBinary_o_tradeSuccess_daily_adaptive(int barThreshold);
double SVM_Prediction_i_returnBinary_o_returnBinary_adaptive();
double SVM_Prediction_i_returnBinary_o_tradeSuccess_daily(double atrThreshold, int atrPeriod, int learningPeriod, int barsUsed, int barThreshold);
double SVM_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed);
double KNN_Prediction_i_returnBinary_o_tradeSuccess_daily(double atrThreshold, int atrPeriod, int learningPeriod, int barsUsed, int barThreshold);
double KNN_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed);
double LM_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed);
double NN_Prediction_i_simpleReturn_o_simpleReturn(int learningPeriod, int barsUsed, int training, int functionType);
double NN_Prediction_i_simpleReturn_o_mfemaeDifference_daily(int learningPeriod, int barsUsed, int training, int functionType);
double NN_Prediction_i_simpleReturn_o_mfemaeDifference(int learningPeriod, int barsUsed, int hourFilter, int training, int barThreshold);
double DeepLearning_NN_Prediction_i_simpleReturn_o_tradeSuccess_daily(int learningPeriod, int barsUsed, int training, int barThreshold);
double LR_Prediction_i_simpleReturn_o_tradeOutcome(int learningPeriod, int barsUsed, int tradingHour, int BE, double initial_SL, int frontier, double minStop, int dsl_type);
void LR_Prediction_i_simpleReturn_o_tradeOutcomeSLTP(int learningPeriod, int barsUsed, int tradingHour, double SL, double TP, int frontier, int *results);
double NN_Prediction_i_simpleReturn_o_tradeOutcome(int learningPeriod, int barsUsed, int tradingHour, int BE, double initial_SL, int frontier, double minStop, int dsl_type, int trainingEpochs);

/**
  * This functions returns a prediction (go long 1, go short -1) using a linear map
  * classifier, 1/0 as inputs for every bearish/bullish bar in each example and
  * the success of a trade (ability to reach the ATR(20) within batThreshold).
  * Two classifiers are trained (one for long and one for short trade success) and 
  * decisions are made depending on their agreement.
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @param int hourFilter
  *   hour in which trading is allowed. Only examples that make decisions on this hour are trained.
  *
  * @param int barThreshold
  *   number of bars to consider for trade success output. For example if a value of 24 is used the output
  *   will be 1 if a positive outcome (a reaching of the ATR) is made within 24 hours, it will be 0 if a negative
  *   or no positive outcome is reached. 
  *
  * @return double
  *	go long/go short prediction (1/-1). Zero gives no prediction.
  * 
  */
double LM_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, int hourFilter, int barThreshold);

/**
  * This functions returns a prediction (go long 1, go short -1) using a support vector machine
  * classifier using a gaussian kernel, 1/0 as inputs for every bearish/bullish bar in each example and
  * the success of a trade (ability to reach the ATR(20) within batThreshold).
  * Two classifiers are trained (one for long and one for short trade success) and 
  * decisions are made depending on their agreement.
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @param double C
  *   An SVM parameter representing how astringent training should be. Try values of 1, 10, 100.
  *
  * @param double gamma
  *   An SVM parameter controling the Gaussian kernel function. Try values of 0.01, 0.1, 1
  *
  * @param int hourFilter
  *   hour in which trading is allowed. Only examples that make decisions on this hour are trained.
  *
  * @param int barThreshold
  *   number of bars to consider for trade success output. For example if a value of 24 is used the output
  *   will be 1 if a positive outcome (a reaching of the ATR) is made within 24 hours, it will be 0 if a negative
  *   or no positive outcome is reached. 
  *
  * @return double
  *	go long/go short prediction (1/-1). Zero gives no prediction.
  * 
  */
double SVM_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, double C, double gamma, int tradingHour, int barThreshold);

/**
  * This functions returns a prediction (go long 1, go short -1) using a neural network
  * classifier, 1/0 as inputs for every bearish/bullish bar in each example and
  * the success of a trade (ability to reach the ATR(20) within batThreshold).
  * Two classifiers are trained (one for long and one for short trade success) and 
  * decisions are made depending on their agreement.
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @param int hourFilter
  *   hour in which trading is allowed. Only examples that make decisions on this hour are trained.
  *
  * @param int barThreshold
  *   number of bars to consider for trade success output. For example if a value of 24 is used the output
  *   will be 1 if a positive outcome (a reaching of the ATR) is made within 24 hours, it will be 0 if a negative
  *   or no positive outcome is reached. 
  *
  * @return double
  *	go long/go short prediction (1/-1). Zero gives no prediction.
  * 
  */
double NN_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, int tradingHour, int barThreshold);

/**
  * This functions returns a prediction (go long 1, go short -1) using a nearest neighbor
  * classifier, 1/0 as inputs for every bearish/bullish bar in each example and
  * the success of a trade (ability to reach the ATR(20) within batThreshold).
  * Two classifiers are trained (one for long and one for short trade success) and 
  * decisions are made depending on their agreement.
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @param int hourFilter
  *   hour in which trading is allowed. Only examples that make decisions on this hour are trained.
  *
  * @param int barThreshold
  *   number of bars to consider for trade success output. For example if a value of 24 is used the output
  *   will be 1 if a positive outcome (a reaching of the ATR) is made within 24 hours, it will be 0 if a negative
  *   or no positive outcome is reached. 
  *
  * @return double
  *	go long/go short prediction (1/-1). Zero gives no prediction.
  * 
  */
double KNN_Prediction_i_returnBinary_o_tradeSuccess(int learningPeriod, int barsUsed, int tradingHour, int barThreshold);

/**
  * This functions returns a prediction (go long 1, go short -1) using a random forest
  * classifier, 1/0 as inputs for every bearish/bullish bar in each example and
  * whether the next bar is bearish or bullish 1/0 as output.
  * A single classifier is trained to give both long/short answers. There is no neutral
  * answer for a binary classifier as a long/short decision is always made.
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @param int hourFilter
  *   hour in which trading is allowed. Only examples that make decisions on this hour are trained.
  *
  * @return double
  *	go long/go short prediction (1/-1).
  * 
  */
double RF_Prediction_i_returnBinary_o_returnBinary(int learningPeriod, int barsUsed);

/**
  * This functions returns a prediction using a linear
  * regression model, simple returns of past bars are used as inputs and 
  * the value of the next bar's predicted simple return is given as output
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @param int hourFilter
  *   hour in which trading is allowed. Only examples that make decisions on this hour are trained.
  *
  * @return double
  *	next bar's simple return (% of last bar's open, positive for bullish, negative for bearish).
  * 
  */
double LR_Prediction_i_simpleReturn_o_simpleReturn(int learningPeriod, int barsUsed);

/**
  * This functions returns a prediction for volatility
  * using a random forrest model. The function uses the volatility of past
  * bars as input (high to low distance) and the volatility of the next bar
  * as output.
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @return double
  *	next bar's predicted volatility
  * 
  */
double RF_Volatility_Prediction(int learningPeriod, int barsUsed);

/**
  * This functions returns a prediction (go long 1, go short -1) using a random forest
  * regression, simple returns as inputs for every bearish/bullish bar in each example and
  * the MFE-MAE difference of each trade as output.
  * Two regressors are trained (one for long and one for short trade success) and 
  * decisions are made depending on their agreement.
  *
  * @param int learningPeriod
  *   number of examples to use for training
  *
  * @param int barsUsed
  *   number of bars to use for training on each example.
  *
  * @param int hourFilter
  *   hour in which trading is allowed. Only examples that make decisions on this hour are trained.
  *
  * @param int barThreshold
  *   number of bars to consider for trade success output. For example if a value of 24 is used the output
  *   will be 1 if a positive outcome (a reaching of the ATR) is made within 24 hours, it will be 0 if a negative
  *   or no positive outcome is reached. 
  *
  * @return double
  *	go long/go short prediction (1/-1). Zero gives no prediction.
  * 
  */
double RF_Prediction_i_simpleReturn_o_mfemaeDifference(int learningPeriod, int barsUsed, int tradingHour, int barThreshold);

double LR_Prediction_i_simpleReturn_o_mfemaeDifference(int learningPeriod, int barsUsed, int tradingHour, int barThreshold);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ASIRIKUY_MACHINE_LEARNING_C_WRAPPER_HPP_ */
