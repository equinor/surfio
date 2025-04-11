#include "include/irap_import.h"
#include "mmap_helper/mmap_helper.h"
#include <algorithm>
#include <bit>
#include <format>
#include <ranges>
#include <span>

template <typename T> inline T swap_byte_order(const T& value) {
  auto tmp = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
  std::ranges::reverse(tmp);
  T result = std::bit_cast<T>(tmp);
  return result;
}

// convert from Fortran order to C order (column major to row major order)
inline int column_major_to_row_major_index(
    size_t f_order, size_t row_size, size_t column_size
) {
  return f_order / row_size + (f_order % row_size) * column_size;
}

template <typename T>
concept NumericType = std::integral<T> || std::floating_point<T>;

template <NumericType T>
inline const char*
read_32bit_value(const char* begin, const char* end, T& value) {
  if (begin + 4 > end)
    throw std::length_error("End of file reached unexpectedly");

  constexpr bool is_integral = std::is_integral<T>::value;

  if constexpr (std::endian::native == std::endian::little)
    if constexpr (is_integral)
      value = swap_byte_order(reinterpret_cast<const int32_t&>(*begin));
    else
      value = swap_byte_order(reinterpret_cast<const float&>(*begin));
  else if constexpr (is_integral)
    value = reinterpret_cast<const int32_t&>(begin);
  else
    value = reinterpret_cast<const float&>(begin);

  return begin + 4;
}

std::tuple<irap_header, const char*>
get_header_binary(std::span<const char> buffer) {
  // header is 100 bytes. 6 guards (24 bytes), 19 values (76 bytes)
  if (buffer.size() < 100)
    throw std::length_error("Header must be at least 100 bytes long");
  irap_header header;
  int32_t dummy;
  int32_t guard;
  float fdummy;
  auto ptr = &*buffer.begin();
  auto end = &*buffer.end();
  try {
    ptr = read_32bit_value(ptr, end, guard);
    if (guard != 32)
      throw std::domain_error("Incorrect block size. Expected 32.");
    ptr = read_32bit_value(ptr, end, dummy);
    if (dummy != -996)
      throw std::domain_error(
          std::format("First value in file should be -996, not {}", dummy)
      );
    ptr = read_32bit_value(ptr, end, header.ny);
    ptr = read_32bit_value(ptr, end, header.xori);
    ptr = read_32bit_value(ptr, end, header.xmax);
    ptr = read_32bit_value(ptr, end, header.yori);
    ptr = read_32bit_value(ptr, end, header.ymax);
    ptr = read_32bit_value(ptr, end, header.xinc);
    ptr = read_32bit_value(ptr, end, header.yinc);
    ptr = read_32bit_value(ptr, end, guard);
    if (guard != 32)
      throw std::domain_error("Block size mismatch");
    ptr = read_32bit_value(ptr, end, guard);
    if (guard != 16)
      throw std::domain_error("Incorrect block size. Expected 16.");
    ptr = read_32bit_value(ptr, end, header.nx);
    ptr = read_32bit_value(ptr, end, header.rot);
    ptr = read_32bit_value(ptr, end, header.xrot);
    ptr = read_32bit_value(ptr, end, header.yrot);
    ptr = read_32bit_value(ptr, end, guard);
    if (guard != 16)
      throw std::domain_error("Block size mismatch");
    ptr = read_32bit_value(ptr, end, guard);
    if (guard != 28)
      throw std::domain_error("Incorrect block size. Expected 28.");
    ptr = read_32bit_value(ptr, end, fdummy);
    ptr = read_32bit_value(ptr, end, fdummy);
    ptr = read_32bit_value(ptr, end, dummy);
    ptr = read_32bit_value(ptr, end, dummy);
    ptr = read_32bit_value(ptr, end, dummy);
    ptr = read_32bit_value(ptr, end, dummy);
    ptr = read_32bit_value(ptr, end, dummy);
    ptr = read_32bit_value(ptr, end, guard);
    if (guard != 28)
      throw std::domain_error("Block size mismatch");
  } catch (const std::exception& e) {
    throw std::domain_error(
        std::format("Failed to read irap headers: {}", e.what())
    );
  }
  return {header, ptr};
}

std::vector<float>
get_values_binary(const char* start, const char* end, int nx, int ny) {
  const size_t nvalues = nx * ny;
  auto values = std::vector<float>(nvalues);
  auto ptr = start;

  int32_t guard_start, guard_end;
  for (auto i = 0u; i < nvalues;) {
    ptr = read_32bit_value(ptr, end, guard_start);
    size_t values_left = guard_start / 4;
    for (size_t j = 0u; j < values_left; ++j, ++i) {
      float value;
      ptr = read_32bit_value(ptr, end, value);
      auto ic = column_major_to_row_major_index(i, nx, ny);
      values[ic] = value;
    }
    ptr = read_32bit_value(ptr, end, guard_end);

    if (guard_start != guard_end) {
      throw std::domain_error("Block size mismatch");
    }
  }

  return values;
}

irap import_irap_binary(std::string path) {
  auto buffer = mmap_file(path);
  auto [header, ptr] = get_header_binary(buffer);
  auto values = get_values_binary(ptr, buffer.end(), header.nx, header.ny);

  return {.header = header, .values = std::move(values)};
}

irap import_irap_binary_from_buffer(std::span<const char> buffer) {
  auto buffer_end = &*buffer.begin() + buffer.size();
  auto [header, ptr] = get_header_binary(buffer);
  auto values = get_values_binary(ptr, buffer_end, header.nx, header.ny);

  return {.header = header, .values = std::move(values)};
}
