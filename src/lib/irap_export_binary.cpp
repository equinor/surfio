#include "include/irap.h"
#include "include/irap_export.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <fstream>
#include <sstream>

template <IsLittleEndianNumeric T> void write_32bit_binary_value(std::ostream& out, T&& value) {
  std::array<char, 4> tmp;
  if constexpr (std::is_integral_v<std::decay_t<T>>)
    tmp = std::bit_cast<std::array<char, 4>>(static_cast<int32_t>(value));
  else
    tmp = std::bit_cast<std::array<char, 4>>(static_cast<float>(value));
  std::ranges::reverse(tmp);
  out.write(&tmp[0], 4);
}

template <IsLittleEndianNumeric... T>
void write_32bit_binary_values(std::ostream& out, T&&... values) {
  (write_32bit_binary_value(out, std::forward<T>(values)), ...);
}

void write_header_binary(const irap_header& header, std::ostream& out) {
  write_32bit_binary_values(
      out, 32, irap_header::id, header.nrow, header.xori, header.xmax, header.yori, header.ymax,
      header.xinc, header.yinc, 32, 16, header.ncol, header.rot, header.xrot, header.yrot, 16, 28,
      0.f, 0.f, 0, 0, 0, 0, 0, 28
  );
}

void write_values_binary(surf_span values, std::ostream& out) {
  size_t written_on_line = 0;
  size_t chunk_length = 0;
  auto rows = values.extent(0);
  auto cols = values.extent(1);
  auto len = values.size();
  for (size_t j = 0; j < cols; j++) {
    for (size_t i = 0; i < rows; i++) {
      if (written_on_line == 0) {
        chunk_length = std::min(len - (j * cols + i), PER_LINE_BINARY);
        write_32bit_binary_value(out, chunk_length * 4);
      }

      auto v = values(i, j);
      write_32bit_binary_value(out, std::isnan(v) ? UNDEF_MAP_IRAP : v);

      if (++written_on_line == chunk_length) {
        write_32bit_binary_value(out, chunk_length * 4);
        written_on_line = 0;
      }
    }
  }
}

void export_irap_to_binary_file(
    const std::string& filename, const irap_header& header, surf_span values
) {
  std::ofstream out(filename, std::ios::binary);
  write_header_binary(header, out);
  write_values_binary(values, out);
}

void export_irap_to_binary_file(const std::string& filename, const irap& data) {
  export_irap_to_binary_file(
      filename, data.header, surf_span{data.values.data(), data.header.ncol, data.header.nrow}
  );
}

std::string export_irap_to_binary_string(const irap_header& header, surf_span values) {
  std::ostringstream out;
  write_header_binary(header, out);
  write_values_binary(values, out);
  return out.str();
}

std::string export_irap_to_binary_string(const irap& data) {
  return export_irap_to_binary_string(
      data.header, surf_span{data.values.data(), data.header.ncol, data.header.nrow}
  );
}
