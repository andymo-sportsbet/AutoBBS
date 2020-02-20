#pragma once

#ifndef KEYK_H_
#define KEYK_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* Runs the KeyK trading strategy.
	*
	* @param StrategyParams* pParams
	*   The structure containing all necessary parameters.
	*
	* @return enum AsirikuyReturnCode
	*   An enum indicating success or the type of failure that occured.
	*/
	AsirikuyReturnCode runKeyK(StrategyParams* pParams);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KeyK_H_ */
