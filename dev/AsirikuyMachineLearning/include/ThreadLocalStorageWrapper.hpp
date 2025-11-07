#ifndef THREAD_LOCAL_STORAGE_WRAPPER_HPP_
#define THREAD_LOCAL_STORAGE_WRAPPER_HPP_
#pragma once

#ifndef STORAGE_UNITS_HPP_
  #include "storageUnits.hpp"
#endif

void initMachineLearningThreadLocalStorage();
inputOutputContainer* getInputOutputContainer();

#endif // THREAD_LOCAL_STORAGE_WRAPPER_HPP