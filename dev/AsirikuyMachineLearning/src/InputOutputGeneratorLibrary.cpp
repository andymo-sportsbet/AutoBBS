/**
 * @file
 * @brief     Machine learning functions to be called from C functions.
 * 
 * @author    Daniel Fernandez (Initial implementation)
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
#include "ThreadLocalStorageWrapper.hpp"
#include "AsirikuyTime.h"
#include <math.h>
#include "EasyTradeCWrapper.hpp"
#include "AsirikuyMachineLearningCWrapper.hpp"

typedef enum outcomes_t
{
	OUTCOME_NO_ENTRY    = 0,
	OUTCOME_LONG_ENTRY  = 1,
	OUTCOME_SHORT_ENTRY = 2

} outcomes;

typedef enum dslTypes_t
{
  DSL_NONE = 0,
  DSL_LINEAR = 1,
  DSL_LOG = 2,
  DSL_PARABOLIC = 3,
  DSL_SQUARE = 4,
  DSL_AFMTL = 5
} dslTypes;



// shark general includes
#include <shark/Data/Dataset.h>

using namespace shark;
using namespace std;

LabeledData<RealVector,unsigned int> p_classification_i_simpleReturn(int barsUsed){

	vector<RealVector> inputs(1,RealVector(barsUsed));
	vector<unsigned int> labels(1);
	int j;

	for(j=0;j<barsUsed;j++){
         inputs[0](j) = (iOpen(0,j)-iOpen(0,1+j))/iOpen(0,1+j);
	}

         labels[0] = 0;
	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}

LabeledData<RealVector,unsigned int>  classification_i_simpleReturn_o_mlemse(int period, int barsUsed, int frontier){
    vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);

	double mle = 0;
	double mse =  0;
	int tl=frontier;
    int hourFilter = getHourShift(PRIMARY_RATES, 0) ;
    int hour;
      
	int i = 0, j = 0, n=0, m = 0;
	while(m<period){

        hour = getHourShift(PRIMARY_RATES, i+tl) ;

		if (hour == hourFilter){		

		mle = 0;
	    mse =  0;

		for(n=0;n<tl;n++){		

			if((high(i+tl-n)-cOpen(i+tl) > mle)){
				mle = (high(i+tl-n)-cOpen(i+tl)) ;
			}

			if((cOpen(i+tl)-low(i+tl-n) > mse)){
				mse = (cOpen(i+tl)-low(i+tl-n)) ;
			}

		}

		 for(j=0;j<barsUsed;j++){
		 inputs[m](j) = (cOpen(tl+j+i)-cOpen(1+tl+j+i))/cOpen(1+tl+j+i);
		 }

         if (mle > mse){
            labels[m] = 0.0;
         } else {
             labels[m] = 1.0;
         }

		 m++;

		}
		i++;		       
	}
	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}

RegressionDataset regression_i_simpleReturn_o_mlemse(int period, int barsUsed, int frontier){

	Data<RealVector> inputs(period,RealVector(barsUsed));
	Data<RealVector> labels(period,RealVector(2));

	double mle = 0;
	double mse =  0;
	int tl=frontier;
    int hourFilter = getHourShift(PRIMARY_RATES, 0) ;
    int hour;
      
	int i = 0, j = 0, n=0, m = 0;
	while(m<period){

        hour = getHourShift(PRIMARY_RATES, i+tl) ;

		if (hour == hourFilter){		

		mle = 0;
	    mse =  0;

		for(n=0;n<tl;n++){		

			if((high(i+tl-n)-cOpen(i+tl) > mle)){
				mle = (high(i+tl-n)-cOpen(i+tl)) ;
			}

			if((cOpen(i+tl)-low(i+tl-n) > mse)){
				mse = (cOpen(i+tl)-low(i+tl-n)) ;
			}

		}

		 for(j=0;j<barsUsed;j++){
		 inputs.element(m)[j] = (cOpen(tl+j+i)-cOpen(1+tl+j+i))/cOpen(1+tl+j+i);
		 }

		 labels.element(m)[0] = mle/cOpen(tl+i);
		 labels.element(m)[1] = mse/cOpen(tl+i);

		 m++;

		}

		i++;		
         
	}
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}

RegressionDataset regression_i_simpleReturn_o_tradeOutcomeSLTP(int period, int barsUsed, int hourFilter, double SL, double TP, int frontier){

   Data<RealVector> inputs(period,RealVector(barsUsed));
   Data<RealVector> labels(period,RealVector(2));

    double openPrice;
    double SL_long, SL_short, TP_long, TP_short;
    double atr;
    int tl = frontier;
    int hour;

   int i = 0, j = 0, n=0, m = 0;
   while(m<period){
        
      hour = getHourShift(PRIMARY_RATES, i+tl) ;
       
      if (hour == hourFilter){      

        atr = iAtrWholeDaysSimpleShift(PRIMARY_RATES, 20, i+tl+1); 
        openPrice = cOpen(i+tl);
        SL_long = openPrice+spread()-atr*SL;
        SL_short = openPrice+atr*SL;
        TP_long = openPrice+spread()+atr*TP;
        TP_short = openPrice-atr*TP;
        
        n = 0;
        
        while (((SL_long < low(i+tl-n)) && (TP_long > high(i+tl-n))) && (n<tl)){
            n += 1;
        }
        
        if((TP_long <= high(i+tl-n)) && (SL_long < low(i+tl-n))){
            labels.element(m)[0] = TP_long-(openPrice+spread());
        } else {
            if(SL_long >= low(i+tl-n)) {
            labels.element(m)[0] = SL_long-(openPrice+spread());
            } else {
                i++;
                continue;
            }
        }
        
        n = 0;
        
        while (((SL_short > high(i+tl-n)+spread()) && (TP_short < low(i+tl-n)+spread()) )&& (n<tl)){
            n += 1;
        }
        
        if((TP_short >= low(i+tl-n)+spread()) && (SL_short > high(i+tl-n)+spread())){
            labels.element(m)[1] = openPrice-TP_short;
        } else {
            if((SL_short <= high(i+tl-n)+spread())){
            labels.element(m)[1] = openPrice-SL_short;
            } else {
                i++;
                continue;
            }
        }
               
       for(j=0;j<barsUsed;j++){
       inputs.element(m)[j] = (iOpen(0,tl+j+i)-iOpen(0,1+tl+j+i))/iOpen(0,1+tl+j+i);
        }
        
       m++;
      }
      
      i++;
   }
   
   RegressionDataset dataset(inputs,labels);
   return dataset;
}

RegressionDataset regression_i_simpleReturn_o_tradeOutcome(int period, int barsUsed, int hourFilter, int BE, double initial_SL, int frontier, double minStop, int dsl_type){

    int m = 0;
    
    initMachineLearningThreadLocalStorage();
    
    inputOutputContainer* mainInputOutputContainer;
    mainInputOutputContainer = getInputOutputContainer();
    Data<RealVector> inputs(period,RealVector(barsUsed));
    Data<RealVector> labels(period,RealVector(2));
    
   if (mainInputOutputContainer->is_init == TRUE){
       
      inputs = mainInputOutputContainer->inputs_g;
      labels = mainInputOutputContainer->labels_g;
    
    int k=0;
    int s=0;
    
    for(k=(period-1);k>0;k--){
        labels.element(k)[0] = labels.element(k-1)[0];
        labels.element(k)[1] = labels.element(k-1)[1];
        for(s=0;s<barsUsed;s++){
            inputs.element(k)[s] = inputs.element(k-1)[s];
        }
    }
    
   }
   
    double openPrice;
    double SL_long, SL_short, new_SL;
    double atr;
    double sqrt_BE, log_BE, BE2;
    int tl = frontier;
    int hour;
    
    sqrt_BE = sqrt((double)BE);
    BE2 = BE*BE;
    log_BE = log((double)BE);

   int i = 0, j = 0, n=0;
   while(m<period){
       
      if ((mainInputOutputContainer->is_init == TRUE) && (m > 0)){
          break;
      }
        
      hour = getHourShift(PRIMARY_RATES, i+tl) ;
       
      if (hour == hourFilter){      

        atr = iAtrWholeDaysSimpleShift(PRIMARY_RATES, 20, i+tl+1); 
        openPrice = cOpen(i+tl);
        SL_long = openPrice+spread()-atr*initial_SL;
        SL_short = openPrice+atr*initial_SL;
        
        n = 0;
        
        while ((SL_long < low(i+tl-n)) && (n<tl)){
            n += 1;
            atr = iAtrWholeDaysSimpleShift(PRIMARY_RATES, 20, i+tl-n+1); 

			switch ( dsl_type ) {
			case DSL_LINEAR:
			    new_SL  = openPrice+spread() - (initial_SL*atr  - n * (initial_SL*atr / BE));
			break;
			case DSL_LOG:
			    new_SL  = openPrice+spread() - (initial_SL*atr  - log((double)n) * (initial_SL*atr / log_BE));
			break;
			case DSL_PARABOLIC:
			    new_SL  = openPrice+spread() - (initial_SL*atr  - (n*n) * (initial_SL*atr / (BE2)));
			break;
			case DSL_SQUARE:
			    new_SL  = openPrice+spread() - (initial_SL*atr  - sqrt((double)n) * (initial_SL*atr / sqrt_BE));
			break;
			case DSL_AFMTL:
			    new_SL  = cOpen(i+tl-n)+spread() - (initial_SL*atr);
				if(new_SL <= SL_long){
					new_SL = cOpen(i+tl-n);
				}
			break;
			default:
			new_SL = cOpen(i+tl-n);
			break;
			}

            
          if (cOpen(i+tl-n)-new_SL > minStop){
              SL_long = new_SL;
            }
        }
        
        labels.element(m)[0] = SL_long-(openPrice+spread());
        
        n = 0;
        
        while ((SL_short > high(i+tl-n)+spread()) && (n<tl)){
            n += 1;
            atr = iAtrWholeDaysSimpleShift(PRIMARY_RATES, 20, i+tl-n+1); 

			switch ( dsl_type ) {
			case DSL_LINEAR:
			    new_SL  = openPrice + (initial_SL*atr  - n * (initial_SL*atr / BE));
			break;
			case DSL_LOG:
			    new_SL  = openPrice + (initial_SL*atr  - log((double)n) * (initial_SL*atr / log_BE));
			break;
			case DSL_PARABOLIC:
			    new_SL  = openPrice + (initial_SL*atr  - (n*n) * (initial_SL*atr / (BE2)));
			break;
			case DSL_SQUARE:
			    new_SL  = openPrice + (initial_SL*atr  - sqrt((double)n) * (initial_SL*atr / sqrt_BE));
			break;
			case DSL_AFMTL:
			    new_SL  = cOpen(i+tl-n) + (initial_SL*atr);
				if(new_SL >= SL_short){
					new_SL = cOpen(i+tl-n);
				}
			break;
			default:
			new_SL = cOpen(i+tl-n);
			break;
			}

          if (new_SL-cOpen(i+tl-n) > minStop){
              SL_short = new_SL;
            }
        }
              
        labels.element(m)[1] = openPrice-SL_short;
        
       for(j=0;j<barsUsed;j++){
       inputs.element(m)[j] = (iOpen(0,tl+j+i)-iOpen(0,1+tl+j+i))/iOpen(0,1+tl+j+i);
        }
       m++;
      }
      i++;
   }
   
   if (mainInputOutputContainer->is_init == FALSE){
       mainInputOutputContainer->is_init = TRUE;
   } 
   
   mainInputOutputContainer->inputs_g = inputs;
   mainInputOutputContainer->labels_g = labels;
   
   RegressionDataset dataset(inputs,labels);
   return dataset;
}

LabeledData<RealVector,unsigned int>  classification_i_simpleReturn_o_mfemaeComparison(int period, int barsUsed, int hourFilter, int successBarThreshold){

	vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);

	double mle = 0;
	double mse =  0;
	struct tm timeInfo;
	int tl=successBarThreshold;

	int i = 0, j = 0, n=0, m = 0;
	while(m<period){

		safe_gmtime(&timeInfo, openTime(i+tl));

		if (timeInfo.tm_hour == hourFilter){		

		mle = 0;
	    mse =  0;

		for(n=0;n<tl;n++){		

			if((high(i+tl-n)-cOpen(i+tl) > mle)){
				mle = (high(i+tl-n)-cOpen(i+tl)) ;
			}

			if((cOpen(i+tl)-low(i+tl-n) > mse)){
				mse = (cOpen(i+tl)-low(i+tl-n)) ;
			}

		}

		 for(j=0;j<barsUsed;j++){
		 inputs[m](j) = (cOpen(tl+j+i)>=cOpen(1+tl+j+i));
		 }
         
        if (mle > mse) labels[m] = 1; else labels[m] = 0;

		 m++;

		}

		i++;
		
	}
	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}


LabeledData<RealVector,unsigned int> classification_i_returnBinary_o_tradeSuccess_daily_shift(int shift, int period, int barsUsed, int successBarThreshold){

	vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);

	int isLongPositive = 0;
	int isLongNegative = 0;
	double atr;
	int tl=successBarThreshold;
	//char buffer[MAX_FILE_PATH_CHARS] = "";
	//char temp[MAX_FILE_PATH_CHARS] = "";

	int i = 0, j = 0, n=0, m=0;
	while(m < period){

		isLongPositive = 0;
	    isLongNegative = 0;

		atr = iAtr(1,20,i+tl+1+shift)*1.5;

		for(n=0;n<tl;n++){		

			if((high(i+tl-n+shift)-cOpen(i+tl+shift) > atr) && (cOpen(i+tl+shift)-low(i+tl-n+shift) <= atr) ){
				isLongPositive = 1;
				break;
			}

			if(cOpen(i+tl+shift)-low(i+tl-n+shift) >= atr){
				isLongNegative = 1;
				break;
			}
		}

		if ((isLongPositive == 0) && (isLongNegative == 0)){
			i++;
			continue;
		}

		 for (j=0;j<barsUsed;j++){
				inputs[m](j) = (cOpen(i+tl+shift+j) >= cOpen(i+tl+shift+1+j));
				}

			if (isLongNegative == 1) labels[m] = 0;
			if (isLongPositive == 1) labels[m] = 1;

		m++;
		i++;
	     
	}

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"%s", buffer );
	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}

LabeledData<RealVector,unsigned int> classification_i_returnBinary_o_tradeSuccess_daily(double atrThreshold, int atrPeriod, int period, int barsUsed, int successBarThreshold){

	vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);

	int isLongPositive = 0;
	int isLongNegative = 0;
	double atr;
	int tl=successBarThreshold;

	int i = 0, j = 0, n=0, m=0;
	while(m < period){

		isLongPositive = 0;
	    isLongNegative = 0;

		atr = iAtr(1,atrPeriod,i+tl+1);

		for(n=0;n<tl;n++){		

			if((high(i+tl-n)-cOpen(i+tl) > atr*atrThreshold) && (cOpen(i+tl)-low(i+tl-n) <= atr*atrThreshold) ){
				isLongPositive = 1;
				break;
			}

			if(cOpen(i+tl)-low(i+tl-n) >= atr*atrThreshold){
				isLongNegative = 1;
				break;
			}
		}

		if ((isLongPositive == 0) && (isLongNegative == 0)){
			i++;
			continue;
		}

				for (j=0;j<barsUsed;j++){
				inputs[m](j) = (cOpen(i+tl+j) >= cOpen(i+tl+1+j));
				}

			if (isLongNegative == 1) labels[m] = 0;
			if (isLongPositive == 1) labels[m] = 1;

		m++;
		i++;
	     
	}

	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}

LabeledData<RealVector,unsigned int> classification_i_returnBinary_o_tradeSuccess(int period, int barsUsed, int direction, int hourFilter, int successBarThreshold){

	vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);

	int isLongPositive = 0;
	int isShortPositive = 0;
	int isLongNegative = 0;
	int isShortNegative = 0;
	double atr;
	struct tm timeInfo;
	int tl=successBarThreshold;
	//char buffer[MAX_FILE_PATH_CHARS] = "";
	//char temp[MAX_FILE_PATH_CHARS] = "";

	atr = iAtr(1,20,1);

	int i = 0, j = 0, n=0, m = 0;
	while(m<period){

		safe_gmtime(&timeInfo, openTime(i+tl));

		if (timeInfo.tm_hour == hourFilter){		

		isLongPositive = 0;
	    isShortPositive = 0;
	    isLongNegative = 0;
	    isShortNegative = 0;

		for(n=0;n<tl;n++){		

			if((high(i+tl-n)-cOpen(i+tl) > atr) && (cOpen(i+tl)-low(i+tl-n) <= atr) && (direction == BUY)){
				isLongPositive = 1;
				break;
			}

			if((cOpen(i+tl)-low(i+tl-n)> atr) &&  (high(i+tl-n)-cOpen(i+tl) <= atr) && (direction == SELL)){
				isShortPositive = 1;
				break;
			}

			if((high(i+tl-n)-cOpen(i+tl)  > atr) && (direction == SELL)){
				isShortNegative= 1;
				break;
			}

			if(cOpen(i+tl)-low(i+tl-n) > atr && (direction == BUY)){
				isLongNegative = 1;
				break;
			}
		}

		if ((isLongPositive == 0) && (isShortPositive == 0)){
			isShortNegative= 1;
			isLongNegative = 1;
		}

		 for(j=0;j<barsUsed;j++){
		 if (cOpen(i+j+tl)-cOpen(i+1+j+tl) >= 0)
         inputs[m](j) = 1; else  inputs[m](j) = 0;	 
		 //sprintf(temp, ",%d", inputs[m](j));
		 //strcat(buffer, temp);

		 }

       /*  if (isShortNegative && isLongNegative){
			labels[i] = OUTCOME_NO_ENTRY;
		 }*/

		 if (direction == BUY){

		 if (isLongNegative == 1) labels[m] = 0;
		 if (isLongPositive == 1) labels[m] = 1;
		 }

		 if (direction == SELL){

		 if (isShortNegative == 1) labels[m] = 0;
		 if (isShortPositive == 1) labels[m] = 1;
		 }

		 //sprintf(temp, ",%d", labels[m]);
		 //strcat(buffer, temp);

		 m++;

		}

		i++;
		     
	}

	//pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"%s", buffer );
	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}

RegressionDataset regression_i_simpleReturn_o_mfemaeDifference_daily(int period, int barsUsed, int successBarThreshold){

	Data<RealVector> inputs(period,RealVector(barsUsed));
	Data<RealVector> labels(period,RealVector(2));

	double mle = 0;
	double mse =  0;
	int tl=successBarThreshold;

	int i = 0, j = 0, n=0, m = 0;
	while(m<period){	

		mle = 0;
	    mse =  0;

		for(n=0;n<tl;n++){		

			if((high(i+tl-n)-cOpen(i+tl) > mle)){
				mle = (high(i+tl-n)-cOpen(i+tl)) ;
			}

			if((cOpen(i+tl)-low(i+tl-n) > mse)){
				mse = (cOpen(i+tl)-low(i+tl-n)) ;
			}

		}

		 for(j=0;j<barsUsed;j++){
		 inputs.element(m)[j] = (cOpen(tl+j+i)-cOpen(1+tl+j+i));
		 }

       /*  if (isShortNegative && isLongNegative){
			labels[i] = OUTCOME_NO_ENTRY;
		 }*/

		 labels.element(m)[0] = mle;
		 labels.element(m)[1] = mse;

		 //pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"training = %lf, %lf, m = %d", mle/100, mse/100, m  );


		 m++;

		i++;
		       
	}
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}

RegressionDataset regression_i_simpleReturn_o_mfemaeDifference(int period, int barsUsed, int hourFilter, int successBarThreshold){

	Data<RealVector> inputs(period,RealVector(barsUsed));
	Data<RealVector> labels(period,RealVector(2));

	double mle = 0;
	double mse =  0;
	struct tm timeInfo;
	int tl=successBarThreshold;

	int i = 0, j = 0, n=0, m = 0;
	while(m<period){

		safe_gmtime(&timeInfo, openTime(i+tl));

		if (timeInfo.tm_hour == hourFilter){		

		mle = 0;
	    mse =  0;

		for(n=0;n<tl;n++){		

			if((high(i+tl-n)-cOpen(i+tl) > mle)){
				mle = (high(i+tl-n)-cOpen(i+tl)) ;
			}

			if((cOpen(i+tl)-low(i+tl-n) > mse)){
				mse = (cOpen(i+tl)-low(i+tl-n)) ;
			}

		}

		 for(j=0;j<barsUsed;j++){
		 inputs.element(m)[j] = (cOpen(tl+j+i)-cOpen(1+tl+j+i))/cOpen(1+tl+j+i);
		 }

       /*  if (isShortNegative && isLongNegative){
			labels[i] = OUTCOME_NO_ENTRY;
		 }*/

		 labels.element(m)[0] = mle/cOpen(tl+i);
		 labels.element(m)[1] = mse/cOpen(tl+i);

		 //pantheios_logprintf(PANTHEIOS_SEV_EMERGENCY, (PAN_CHAR_T*)"training = %lf, %lf, m = %d", mle/100, mse/100, m  );


		 m++;

		}

		i++;
		

         
	}
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}

LabeledData<RealVector,unsigned int> classification_i_ma_o_returnBinary(int period){

   vector<RealVector> inputs(period,RealVector(7));
   vector<unsigned int> labels(period);

   int i = 0;
   for(i=0;i<period;i++){

         inputs[i](0) = cClose(i+2) > cOpen(i+2);
       inputs[i](1) = iMA(3,PRIMARY_RATES,5,i+2) > iMA(3,PRIMARY_RATES,10,i+2) ;
       inputs[i](2) = iMA(3,PRIMARY_RATES,10,i+2) > iMA(3,PRIMARY_RATES,15,i+2) ;
       inputs[i](3) = iMA(3,PRIMARY_RATES,15,i+2) > iMA(3,PRIMARY_RATES,20,i+2) ;
       inputs[i](4) = iMA(3,PRIMARY_RATES,20,i+2) > iMA(3,PRIMARY_RATES,25,i+2) ;
       inputs[i](5) = iMA(3,PRIMARY_RATES,25,i+2) > iMA(3,PRIMARY_RATES,30,i+2) ;
       inputs[i](6) = iMA(3,PRIMARY_RATES,30,i+2) > iMA(3,PRIMARY_RATES,35,i+2) ;

         if (cClose(i+1)-cOpen(i+1) >= 0)
         labels[i] = 1; else labels[i] = 0;
         
   }
   
   ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
   return dataset;
}

LabeledData<RealVector,unsigned int> p_classification_i_ma(){

   vector<RealVector> inputs(1,RealVector(7));
   vector<unsigned int> labels(1);

         inputs[0](0) = cClose(1) > cOpen(1);
       inputs[0](1) = iMA(3,PRIMARY_RATES,5,1) > iMA(3,PRIMARY_RATES,10,1);
       inputs[0](2) = iMA(3,PRIMARY_RATES,10,1) > iMA(3,PRIMARY_RATES,15,1);
       inputs[0](3) = iMA(3,PRIMARY_RATES,15,1) > iMA(3,PRIMARY_RATES,20,1);
       inputs[0](4) = iMA(3,PRIMARY_RATES,20,1) > iMA(3,PRIMARY_RATES,25,1);
       inputs[0](5) = iMA(3,PRIMARY_RATES,25,1) > iMA(3,PRIMARY_RATES,30,1);
       inputs[0](6) = iMA(3,PRIMARY_RATES,30,1) > iMA(3,PRIMARY_RATES,35,1);

         labels[0] = 0;
   
   ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
   return dataset;
}

RegressionDataset regression_i_simpleReturn_o_simpleReturn(int period, int barsUsed){

	Data<RealVector> inputs(period,RealVector(barsUsed));
	Data<RealVector> labels(period,RealVector(1));
	
	int  i = 0, j = 0;
	for(i=0;i<period;i++){
			
				for (j=0;j<barsUsed;j++){
				inputs.element(i)[j] = (cOpen(1+i+j)-cOpen(2+j+i))/cOpen(2+j+i);
				}


				labels.element(i)[0] = (cOpen(i)-cOpen(i+1))/cOpen(i+1);
	}
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}

RegressionDataset regression_i_simpleReturn_o_simpleReturn_shift(int shift, int period, int barsUsed){

	Data<RealVector> inputs(period,RealVector(barsUsed));
	Data<RealVector> labels(period,RealVector(1));
	
	int  i = 0, j = 0;
	for(i=0;i<period;i++){
			
				for (j=0;j<barsUsed;j++){
				inputs.element(i)[j] = (cOpen(1+i+j+shift)-cOpen(2+j+i+shift));
				}


				labels.element(i)[0] = (cOpen(i+shift)-cOpen(i+1+shift));
	}
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}


RegressionDataset p_regression_i_simpleReturn(int barsUsed){

	Data<RealVector> inputs(1,RealVector(barsUsed));
	Data<RealVector> labels(1,RealVector(1));
	int j = 0, k=0;

		for (j=0;j<barsUsed;j++){
         inputs.element(0)[j] = (cOpen(j)-cOpen(j+1))/cOpen(j+1);
		}


	labels.element(0)[0] = 0;
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}

RegressionDataset p_regression_i_simpleReturn_shift(int shift, int barsUsed){

	Data<RealVector> inputs(1,RealVector(barsUsed));
	Data<RealVector> labels(1,RealVector(1));
	int j = 0, k=0;

		for (j=0;j<barsUsed;j++){
         inputs.element(0)[j] = (cOpen(j+shift)-cOpen(j+1+shift));
		}


	labels.element(0)[0] = 0;
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}

LabeledData<RealVector,unsigned int> p_classification_i_returnBinary(int barsUsed){

	vector<RealVector> inputs(1,RealVector(barsUsed));
	vector<unsigned int> labels(1);
	int j;

	for(j=0;j<barsUsed;j++){
         inputs[0](j) = (iOpen(0,j) >= iOpen(0,1+j));
	}

         labels[0] = 0;
	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}


LabeledData<RealVector,unsigned int> classification_i_returnBinary_o_returnBinary(int period, int barsUsed){

	vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);
	
	int i = 0;
	int j = 0;
	int lag = 0;

	for(i=0;i<period;i++){

				for (j=lag;j<barsUsed+lag;j++){
				inputs[i](j-lag) = (iOpen(0,i+1+j) >= iOpen(0,i+2+j));
				}

				labels[i] = (cOpen(i) >= cOpen(i+1+lag));
	}

	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}


LabeledData<RealVector,unsigned int> classification_i_returnBinary_o_returnBinary_shift(int shift, int period, int barsUsed){

	vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);
	
	int i = 0;
	int j = 0;
	int lag = 4;

	for(i=0;i<period;i++){

				for (j=lag;j<barsUsed+lag;j++){
				inputs[i](j-lag) = (iOpen(0,i+1+j+shift) >= iOpen(0,i+2+j+shift));
				}

				labels[i] = (cOpen(i+shift) >= cOpen(i+1+lag+shift));
	}

	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}


LabeledData<RealVector,unsigned int> classification_i_returnBinary_o_returnBinary_direction(int period, int barsUsed, int direction){

	vector<RealVector> inputs(period,RealVector(barsUsed));
	vector<unsigned int> labels(period);
	
	int i = 0;
	int j = 0;
	double atr;
	int lag = 4;

	for(i=0;i<period;i++){

				atr = iAtr(0,5,lag+1+i);

				for (j=lag;j<barsUsed+lag;j++){
				inputs[i](j-lag) = (cOpen(i+1+j) >= cOpen(i+2+j));
				}

				if (direction == BUY)
				labels[i] = (cOpen(i)-cOpen(i+1+lag) >= atr*0.3);

				if (direction == SELL)
				labels[i] = (cOpen(i)-cOpen(i+1+lag) <= -atr*0.3);
	}

	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}


LabeledData<RealVector,unsigned int> p_classification_i_returnBinary_shift(int shift, int barsUsed){

	vector<RealVector> inputs(1,RealVector(barsUsed));
	vector<unsigned int> labels(1);
	int j;

	for(j=0;j<barsUsed;j++){
         inputs[0](j) = (cOpen(j+shift) >= cOpen(1+j+shift));
	}

         labels[0] = 0;
	
	ClassificationDataset dataset = createLabeledDataFromRange(inputs,labels);
	return dataset;
}

RegressionDataset regression_i_range_o_range(int period, int barsUsed){

	Data<RealVector> inputs(period,RealVector(barsUsed));
	Data<RealVector> labels(period,RealVector(1));
	
	int i = 0;
	int j = 0;

	for(i=0;i<period;i++){

				for (j=0;j<barsUsed;j++){
				inputs.element(i)[j] = iHigh(DAILY_RATES,i+1+j)-iLow(DAILY_RATES,i+1+j)/cOpen(2+j+i);
				}

				labels.element(i)[0] = iHigh(DAILY_RATES,i)-iLow(DAILY_RATES,i)/cOpen(i+1);
	}
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
	
}

RegressionDataset p_regression_i_range(int barsUsed){

	Data<RealVector> inputs(1,RealVector(barsUsed));
	Data<RealVector> labels(1,RealVector(1));

	int j = 0;

	for (j=0;j<barsUsed;j++){
         inputs.element(0)[j] = (iHigh(DAILY_RATES,j) - iLow(DAILY_RATES,j))/cOpen(j+1);
	}
    
	labels.element(0)[0] = 0;
	
	RegressionDataset dataset(inputs,labels);
	return dataset;
}