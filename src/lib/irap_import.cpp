#include "include/irap_import.h"
#include "mmap_helper/mmap_helper.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>
#include <locale>
#include <stdexcept>
#include <tuple>
#include <vector>

auto locale = std::locale("C");
auto& facet = std::use_facet<std::ctype<char>>(locale);
auto is_space = [](char ch) { return facet.is(std::ctype_base::space, ch); };

// convert from Fortran order to C order (column major to row major order)
inline int column_major_to_row_major_index(
    size_t f_order, size_t row_size, size_t column_size
) {
  return f_order / row_size + (f_order % row_size) * column_size;
}

template <typename T, typename... U>
const char*
read_headers(const char* start, const char* end, T& arg, U&... args) {
  start = std::find_if_not(start, end, is_space);
  auto [ptr, ec] = std::from_chars(start, end, arg);
  if (ec != std::errc{})
    throw std::domain_error("Failed to read irap headers");

  if constexpr (sizeof...(args) > 0)
    ptr = read_headers(ptr, end, args...);

  return ptr;
}

std::tuple<irap_header, const char*>
get_header(const char* start, const char* end) {
  int magic_header;
  int idum;
  irap_header head;
  auto ptr = read_headers(
      start, end, magic_header, head.ny, head.xinc, head.yinc, head.xori,
      head.xmax, head.yori, head.ymax, head.nx, head.rot, head.xrot, head.yrot,
      idum, idum, idum, idum, idum, idum, idum
  );
  if (magic_header != -996)
    throw std::runtime_error(
        "First value in irap ascii file is incorrect. "
        "Irap ASCII. Expected: -996, got: " +
        std::to_string(magic_header)
    );

  if (head.rot < 0.0)
    head.rot += 360.0;

  if (head.nx < 0 || head.ny < 0)
    throw std::domain_error(
        "Incorrect dimensions encountered while importing Irap ASCII"
    );

  return {head, ptr};
}

std::vector<float>
get_values(const char* start, const char* end, int nx, int ny) {
  const size_t nvalues = nx * ny;
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

    auto result = std::from_chars(start, end, value);
    start = result.ptr;
    if (result.ec != std::errc())
      throw std::domain_error(
          "Failed to read values during Irap ASCII import."
      );

    if (value == UNDEF_MAP_IRAP)
      value = std::numeric_limits<float>::quiet_NaN();

    auto ic = column_major_to_row_major_index(i, nx, ny);
    values[ic] = value;
  }

  return values;
}

irap import_irap_ascii(std::string path) {
  auto buffer = mmap_file(std::move(path));

  auto [head, ptr] = get_header(buffer.begin(), buffer.end());
  auto values = get_values(ptr, buffer.end(), head.nx, head.ny);

  return {.header = std::move(head), .values = std::move(values)};
}

irap import_irap_ascii_string(const std::string& string) {
  auto buffer = string.c_str();
  auto buffer_end = buffer + string.size();
  auto [head, ptr] = get_header(buffer, buffer_end);
  auto values = get_values(ptr, buffer_end, head.nx, head.ny);

  return {.header = std::move(head), .values = std::move(values)};
}
