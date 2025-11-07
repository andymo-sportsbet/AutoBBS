#ifndef THREAD_LOCAL_STORAGE_HPP_
#define THREAD_LOCAL_STORAGE_HPP_
#pragma once

#include "storageUnits.hpp"

class ThreadLocalStorage {
  public:	
    ThreadLocalStorage();
    ~ThreadLocalStorage();
  
	void initMachineLearningThreadLocalStorage();
    inputOutputContainer* getInputOutputContainer();

  private:
    inputOutputContainer  mainInputOutputContainer;
};

#endif // THREAD_LOCAL_STORAGE_HPP_