#pragma once

#include <bit>
#include <vector>

namespace surfio::irap {
constexpr float UNDEF_MAP_IRAP_ASCII = 9999900.f;
constexpr float UNDEF_MAP_IRAP_BINARY = 1e30f;

struct irap_header {
  // All irap headers start with -996
  static constexpr int id = -996;
  // Number of column
  int ncol;
  // Number of rows
  int nrow;
  // x coordinate of origin
  double xori = 0.;
  // y coordinate of origin
  double yori = 0.;
  // The outer edge on the x axis
  // Should always be xori + (ncol - 1) * xinc;
  double xmax = 0.;
  // The outer edge on the y axis
  // Should always be yori + (nrow - 1) * yinc
  double ymax = 0.;
  // How long each x increment is
  double xinc = 1.;
  // How long each y increment is
  double yinc = 1.;
  // Rotation of matrix
  double rot = 0.;
  // x coordinate of rotation point. Must be the same as xori for Irap RMS
  double xrot = 0.;
  // y coordinate of rotation point. Must be the same as yori for Irap RMS
  double yrot = 0.;
  // We do not know what these values signify.
  // They are all 0 in files we have access to.
  // float unknown[2];
  // int more_unknown[5];
  friend bool operator==(irap_header, irap_header) = default;
  friend bool operator!=(irap_header, irap_header) = default;
};

struct irap {
  irap_header header;
  std::vector<float> values;
};

template <typename T>
concept IsLittleEndian = std::endian::native == std::endian::little;
template <typename T>
concept IsLittleEndianNumeric =
    (std::integral<std::decay_t<T>> || std::floating_point<std::decay_t<T>>) && IsLittleEndian<T>;
} // namespace surfio::irap
