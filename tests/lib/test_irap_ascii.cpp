#include "helpers/helper.h"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <irap.h>
#include <irap_export.h>
#include <irap_import.h>

using namespace Catch;

SCENARIO("Verify that surfio can read and write irap ascii files", "[test_irap_ascii.cpp]") {
  auto header = irap_header{
      .ncol = 6000,
      .nrow = 6000,
  };
  auto values = create_random_values(header.ncol * header.nrow);
  auto original = irap{.header = header, .values = values};
  export_irap_to_ascii_file("surf.irap_ascii", original);
  auto imported = import_irap_ascii("surf.irap_ascii");

  CHECK(imported.header == original.header);
  CHECK_THAT(imported.values, Matchers::Approx(original.values).margin(0.001));
}
