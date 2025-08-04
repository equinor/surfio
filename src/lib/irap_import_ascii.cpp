#include "include/irap_import.h"
#include "mmap_wrapper/mmap_wrapper.h"
#include "strto_wrapper/strto_wrapper.h"
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <format>

#include <errno.h>
#include <stdlib.h>

namespace fs = std::filesystem;

namespace surfio::irap {
auto locale = std::locale("C");
auto& facet = std::use_facet<std::ctype<char>>(locale);
auto is_space = [](char ch) { return facet.is(std::ctype_base::space, ch); };

template <typename T, typename... U>
const char* read_headers(const char* start, const char* end, T& arg, U&... args) {
  if constexpr (!STD_FROM_CHARS_FLOAT_SUPPORT && std::is_floating_point_v<T>) {
#if !STD_FROM_CHARS_FLOAT_SUPPORT
    static thread_local wrapped_c_locale wrapped_locale;
    char* ptr;
    if constexpr (std::is_same_v<T, float>)
      arg = strtof_l_wrapper(start, &ptr, wrapped_locale.cLocale);
    else if constexpr (std::is_same_v<T, double>)
      arg = strtod_l_wrapper(start, &ptr, wrapped_locale.cLocale);
    else
      throw std::domain_error("Unsupported floating point type in irap headers");

    if (errno != 0 || ptr == start || ptr > end)
      throw std::domain_error("Failed to read irap headers");
    start = ptr;
#endif
  } else {
    // find first non whitespace char as from_chars does not ignore leading whitespace
    start = std::find_if_not(start, end, is_space);
    auto [ptr, ec] = std::from_chars(start, end, arg);
    if (ec != std::errc{})
      throw std::domain_error("Failed to read irap headers");
    start = ptr;
  }

  if constexpr (sizeof...(args) > 0)
    start = read_headers(start, end, args...);

  return start;
}

std::tuple<irap_header, const char*> get_header(const char* start, const char* end) {
  int magic_header;
  int idum;
  irap_header head;
  auto ptr = read_headers(
      start, end, magic_header, head.nrow, head.xinc, head.yinc, head.xori, head.xmax, head.yori,
      head.ymax, head.ncol, head.rot, head.xrot, head.yrot, idum, idum, idum, idum, idum, idum, idum
  );
  if (magic_header != irap_header::id)
    throw std::runtime_error(
        std::format(
            "First value in irap ascii file is incorrect. "
            "Irap ASCII. Expected: {}, got: {}",
            irap_header::id, std::to_string(magic_header)
        )
    );

  if (head.rot < 0.0)
    head.rot += 360.0;

  if (head.ncol < 0 || head.nrow < 0)
    throw std::domain_error("Incorrect dimensions encountered while importing Irap ASCII");

  return {head, ptr};
}

std::vector<float> get_values(const char* start, const char* end, int ncol, int nrow) {
  const size_t nvalues = ncol * nrow;
  auto values = std::vector<float>(nvalues);
  for (auto i = 0u; i < nvalues; ++i) {
    float value;

    start = std::find_if_not(start, end, is_space);
    if (start == end)
      throw std::length_error(
          std::format(
              "End of file reached before reading all values. Expected: {}, "
              "got {}",
              nvalues, i
          )
      );

#if !STD_FROM_CHARS_FLOAT_SUPPORT
    static thread_local wrapped_c_locale wrapped_locale;
    char* ptr;
    value = strtof_l_wrapper(start, &ptr, wrapped_locale.cLocale);
    if (errno != 0 || ptr == start || ptr > end)
      throw std::domain_error("Failed to read values during Irap ASCII import.");
    start = ptr;
#else
    auto result = std::from_chars(start, end, value);
    start = result.ptr;
    if (result.ec != std::errc())
      throw std::domain_error("Failed to read values during Irap ASCII import.");
#endif
    if (value >= UNDEF_MAP_IRAP_ASCII)
      value = std::numeric_limits<float>::quiet_NaN();

    auto ic = column_major_to_row_major_index(i, ncol, nrow);
    values[ic] = value;
  }

  return values;
}

irap from_ascii_file(const fs::path& file) {
  auto buffer = mmap::mmap_file(file);

  auto [head, ptr] = get_header(buffer.begin(), buffer.end());
  auto values = get_values(ptr, buffer.end(), head.ncol, head.nrow);

  return {.header = std::move(head), .values = std::move(values)};
}

irap from_ascii_string(std::string_view buffer) {
  auto buffer_begin = buffer.data();
  auto buffer_end = buffer_begin + buffer.size();
  auto [head, ptr] = get_header(buffer_begin, buffer_end);
  auto values = get_values(ptr, buffer_end, head.ncol, head.nrow);

  return {.header = std::move(head), .values = std::move(values)};
}
} // namespace surfio::irap
