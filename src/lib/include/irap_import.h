#pragma once

#include "irap.h"
#include <span>
#include <string>

// convert from Fortran order to C order
inline size_t column_major_to_row_major_index(size_t idx, size_t nrow, size_t ncol) {
  return idx / nrow + (idx % nrow) * ncol;
}

irap import_irap_ascii(std::string path);
irap import_irap_ascii_from_string(const std::string& buffer);
irap import_irap_binary(std::string path);
irap import_irap_binary_from_buffer(std::span<const char> buffer);
