#pragma once

#include "irap.h"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

using values_array =
    pybind11::array_t<float, pybind11::array::c_style | pybind11::array::forcecast>;

struct irap_python {
  surfio::irap::irap_header header;
  values_array values;
};
