#include "include/irap_export.h"
#include <cmath>
#include <fstream>
#include <iomanip>

void write_header_ascii(const irap_header& header, std::ostream& out) {
  out << std::setprecision(6) << std::fixed << std::showpoint;
  out << "-996 " << header.ny << " " << header.xinc << " " << header.yinc
      << "\n";
  out << header.xori << " " << header.xmax << " " << header.yori << " "
      << header.ymax << "\n";
  out << header.nx << " " << header.rot << " " << header.xrot << " "
      << header.yrot << "\n";
  out << "0  0  0  0  0  0  0\n";
}

void write_values_ascii(surf_span values, std::ostream& out) {
  out << std::setprecision(4) << std::fixed << std::showpoint;
  size_t written_on_line = 0;
  auto rows = values.extent(0);
  auto cols = values.extent(1);
  for (size_t j = 0; j < cols; j++) {
    for (size_t i = 0; i < rows; i++) {
      auto v = values[i, j];
      if (std::isnan(v))
        out << UNDEF_MAP_IRAP;
      else
        out << v;
      if (++written_on_line < MAX_PER_LINE) {
        out << " ";
      } else {
        out << "\n";
        written_on_line = 0;
      }
    }
  }
}

void export_irap_to_ascii_file(
    const std::string& filename, const irap_header& header, surf_span values
) {
  std::ofstream out(filename);
  write_header_ascii(header, out);
  write_values_ascii(values, out);
}

void export_irap_to_ascii_file(const std::string& filename, const irap& data) {
  export_irap_to_ascii_file(
      filename, data.header,
      surf_span{data.values.data(), data.header.nx, data.header.ny}
  );
}

std::string
export_irap_to_ascii_string(const irap_header& header, surf_span values) {
  std::stringstream out;
  write_header_ascii(header, out);
  write_values_ascii(values, out);
  return out.str();
}

std::string export_irap_to_ascii_string(const irap& data) {
  return export_irap_to_ascii_string(
      data.header, surf_span{data.values.data(), data.header.nx, data.header.ny}
  );
}
