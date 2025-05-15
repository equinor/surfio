#pragma once

#include "irap.h"
#include <filesystem>
#include <span>
#include <string_view>

namespace surfio::irap {
// convert from Fortran order to C order
inline size_t column_major_to_row_major_index(size_t idx, size_t nrow, size_t ncol) {
  return idx / nrow + (idx % nrow) * ncol;
}

irap import_irap_ascii(const std::filesystem::path& file);
irap import_irap_ascii_from_string(std::string_view buffer);
irap import_irap_binary(const std::filesystem::path& file);
irap import_irap_binary_from_buffer(std::span<const char> buffer);
} // namespace surfio::irap
