#pragma once

#ifndef BBS_H_
#define BBS_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* Runs the BBS trading strategy.
	*
	* @param StrategyParams* pParams
	*   The structure containing all necessary parameters.
	*
	* @return enum AsirikuyReturnCode
	*   An enum indicating success or the type of failure that occured.
	*/
	AsirikuyReturnCode runBBS(StrategyParams* pParams);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BBS_H_ */
