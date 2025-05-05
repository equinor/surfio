#include "irap.h"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <irap.h>
#include <irap_export.h>
#include <irap_import.h>
#include <random>

SCENARIO("Verify that surfio can read and write irap ascii files", "[test_irap_ascii.cpp]") {
  auto _header = irap_header{
      .nrow = 6000,
      .xori = 0.,
      .xmax = 0,
      .yori = 0.,
      .ymax = 0.,
      .xinc = 0,
      .yinc = 0.,
      .ncol = 6000,
      .rot = 0.,
      .xrot = 0.,
      .yrot = 0.
  };
  auto _values = std::vector<float>(_header.ncol * _header.nrow);
  std::mt19937 g;
  std::uniform_real_distribution<> u;
  std::generate(_values.begin(), _values.end(), [&]() { return u(g); });

  auto _original = irap{.header = _header, .values = _values};
  export_irap_to_ascii_file("surf.irap", _original);
  auto _imported = import_irap_ascii("surf.irap");

  CHECK(_imported.header == _original.header);
  CHECK(std::ranges::equal(_imported.values, _original.values, [](auto lhs, auto rhs) {
    return (lhs - rhs) < 0.01;
  }));
}
