#include "Precompiled.hpp"

#include "ThreadLocalStorageWrapper.hpp"
#include "ThreadLocalStorage.hpp"

namespace {
  boost::thread_specific_ptr<ThreadLocalStorage> ThreadLocalStoragePtr;
}

void initMachineLearningThreadLocalStorage() {
  if (ThreadLocalStoragePtr.get() == NULL){
        ThreadLocalStoragePtr.reset(new ThreadLocalStorage());
        ThreadLocalStoragePtr->initMachineLearningThreadLocalStorage();
  }
}

inputOutputContainer* getInputOutputContainer() {
  return ThreadLocalStoragePtr->getInputOutputContainer();
}