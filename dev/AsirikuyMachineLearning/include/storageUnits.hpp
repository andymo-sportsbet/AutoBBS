#ifndef STORAGE_UNITS_HPP_
#define STORAGE_UNITS_HPP_
#pragma once

#include <shark/Data/Dataset.h>
using namespace shark;

typedef struct inputOutputContainer_t {
  int is_init;
  Data<RealVector> inputs_g;
  Data<RealVector> labels_g;
} inputOutputContainer;

#endif // STORAGE_UNITS_HPP_