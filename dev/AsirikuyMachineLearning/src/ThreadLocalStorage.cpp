#include "Precompiled.h"

#include "ThreadLocalStorage.hpp"

ThreadLocalStorage::ThreadLocalStorage() {}
ThreadLocalStorage::~ThreadLocalStorage() {}

void ThreadLocalStorage::initMachineLearningThreadLocalStorage() {
  mainInputOutputContainer.is_init = 0;
}

inputOutputContainer* ThreadLocalStorage::getInputOutputContainer() {
  return(&mainInputOutputContainer);
}