#pragma once
#include <bit>
#include <vector>
constexpr float UNDEF_MAP_IRAP = 9999900.0000;

struct irap_header {
  static constexpr int id = -996;
  int nrow;
  double xori;
  double xmax;
  double yori;
  double ymax;
  double xinc;
  double yinc;
  int ncol;
  double rot;
  double xrot;
  double yrot;
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
