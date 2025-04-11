#include "include/irap_export.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <fstream>
#include <sstream>

template <IsLittleEndianNumeric T>
inline void write_32bit_binary_value(T&& value, std::ostream& out) {
  std::array<char, 4> tmp;
  if constexpr (std::is_integral_v<std::decay_t<T>>)
    tmp = std::bit_cast<std::array<char, 4>>(static_cast<int32_t>(value));
  else
    tmp = std::bit_cast<std::array<char, 4>>(static_cast<float>(value));
  std::ranges::reverse(tmp);
  out.write(&tmp[0], 4);
}

void write_header_binary(const irap_header& header, std::ostream& out) {
  write_32bit_binary_value(32, out);
  write_32bit_binary_value(irap_header::id, out);
  write_32bit_binary_value(header.ny, out);
  write_32bit_binary_value(header.xori, out);
  write_32bit_binary_value(header.xmax, out);
  write_32bit_binary_value(header.yori, out);
  write_32bit_binary_value(header.ymax, out);
  write_32bit_binary_value(header.xinc, out);
  write_32bit_binary_value(header.yinc, out);
  write_32bit_binary_value(32, out);
  write_32bit_binary_value(16, out);
  write_32bit_binary_value(header.nx, out);
  write_32bit_binary_value(header.rot, out);
  write_32bit_binary_value(header.xrot, out);
  write_32bit_binary_value(header.yrot, out);
  write_32bit_binary_value(16, out);
  write_32bit_binary_value(28, out);
  write_32bit_binary_value(0.f, out);
  write_32bit_binary_value(0.f, out);
  write_32bit_binary_value(0, out);
  write_32bit_binary_value(0, out);
  write_32bit_binary_value(0, out);
  write_32bit_binary_value(0, out);
  write_32bit_binary_value(0, out);
  write_32bit_binary_value(28, out);
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
        write_32bit_binary_value(chunk_length * 4, out);
      }

      auto v = values(i, j);
      if (std::isnan(v))
        write_32bit_binary_value(UNDEF_MAP_IRAP, out);
      else
        write_32bit_binary_value(v, out);

      if (++written_on_line == chunk_length) {
        write_32bit_binary_value(chunk_length * 4, out);
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
      filename, data.header,
      surf_span{data.values.data(), data.header.nx, data.header.ny}
  );
}

std::string
export_irap_to_binary_string(const irap_header& header, surf_span values) {
  std::ostringstream out;
  write_header_binary(header, out);
  write_values_binary(values, out);
  return out.str();
}

std::string export_irap_to_binary_string(const irap& data) {
  return export_irap_to_binary_string(
      data.header, surf_span{data.values.data(), data.header.nx, data.header.ny}
  );
}
