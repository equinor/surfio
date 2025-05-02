#include "irap.h"
#include <ostream>

#if __cpp_lib_mdspan
#include <mdspan>
using std::dynamic_extent;
using std::extents;
using std::mdspan;
#else
#include <experimental/mdspan>
using std::experimental::dynamic_extent;
using std::experimental::extents;
using std::experimental::mdspan;
#endif

constexpr size_t MAX_PER_LINE = 9; // Maximum accepted by some software
// write 8 values per block. this could make
// it easier to use simd to import the values
constexpr size_t PER_LINE_BINARY = 8;

using surf_span = mdspan<const float, extents<size_t, dynamic_extent, dynamic_extent>>;

void write_header_ascii(const irap_header& header, std::ostream& out);
void write_values_ascii(surf_span values, std::ostream& out);
void write_header_binary(const irap_header& header, std::ostream& out);
void write_values_binary(surf_span values, std::ostream& out);

void export_irap_to_ascii_file(
    const std::string& filename, const irap_header& header, surf_span values
);
void export_irap_to_ascii_file(const std::string& filename, const irap& data);

std::string export_irap_to_ascii_string(const irap_header& header, surf_span values);
std::string export_irap_to_ascii_string(const irap& data);

void export_irap_to_binary_file(
    const std::string& filename, const irap_header& header, surf_span values
);
void export_irap_to_binary_file(const std::string& filename, const irap& data);

std::string export_irap_to_binary_string(const irap_header& header, surf_span values);
std::string export_irap_to_binary_string(const irap& data);
