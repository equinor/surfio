#include "include/irap.h"
#include "include/irap_export.h"
#include <cmath>
#include <format>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <sstream>

// All irap headers start with -996
static const auto id = std::format("{} ", irap_header::id);
static auto UNDEF_MAP_IRAP_STRING = std::format("{:f}", UNDEF_MAP_IRAP);

void write_header_ascii(const irap_header& header, std::ostream& out) {
  out << std::setprecision(6) << std::fixed << std::showpoint;
  out << id << header.nrow << " " << header.xinc << " " << header.yinc << "\n";
  out << header.xori << " " << header.xmax << " " << header.yori << " " << header.ymax << "\n";
  out << header.ncol << " " << header.rot << " " << header.xrot << " " << header.yrot << "\n";
  out << "0 0 0 0 0 0 0\n";
}

void write_values_ascii(surf_span values, std::ostream& out) {
  out << std::setprecision(4) << std::fixed << std::showpoint;
  size_t values_on_current_line = 0;
  auto rows = values.extent(0);
  auto cols = values.extent(1);
  for (size_t j = 0; j < cols; j++) {
    for (size_t i = 0; i < rows; i++) {
      auto v = values(i, j);
      out << (std::isnan(v) ? UNDEF_MAP_IRAP_STRING : std::format("{:f}", v));

      ++values_on_current_line %= MAX_PER_LINE;
      out << (values_on_current_line ? " " : "\n");
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
      filename, data.header, surf_span{data.values.data(), data.header.ncol, data.header.nrow}
  );
}

std::string export_irap_to_ascii_string(const irap_header& header, surf_span values) {
  std::stringstream out;
  write_header_ascii(header, out);
  write_values_ascii(values, out);
  return out.str();
}

std::string export_irap_to_ascii_string(const irap& data) {
  return export_irap_to_ascii_string(
      data.header, surf_span{data.values.data(), data.header.ncol, data.header.nrow}
  );
}
