#ifndef STRATEGY_API_H_
#define STRATEGY_API_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Centralized prototypes for common strategy helper functions to avoid
 * implicit function declarations and duplicated forward declarations
 * scattered across strategy implementation files.
 * Add new shared prototypes here as they are introduced.
 */

typedef struct StrategyParams StrategyParams; /* Forward decl */

/* TestEA helpers */
AsirikuyReturnCode loadIndicators(StrategyParams* pParams, void* pIndicators);
AsirikuyReturnCode setUIValues(StrategyParams* pParams, void* pIndicators);
AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, void* pIndicators);
AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, void* pIndicators);

/* Common time utility provided externally */
void safe_timeString(char* dest, time_t t);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STRATEGY_API_H_ */
