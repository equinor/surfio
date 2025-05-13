#include "helpers/helper.h"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <filesystem>
#include <irap.h>
#include <irap_export.h>
#include <irap_import.h>

using namespace Catch;
namespace fs = std::filesystem;

SCENARIO("Verify that surfio can read and write irap binary files", "[test_irap_binary.cpp]") {
  const std::string filename("surf.irap");
  auto header = irap_header{
      .ncol = 6000,
      .nrow = 6000,
  };
  auto values = create_random_values(header.ncol * header.nrow);
  auto original = irap{.header = header, .values = values};
  export_irap_to_binary_file(filename, original);
  auto imported = import_irap_binary(filename);

  CHECK(imported.header == original.header);
  CHECK_THAT(imported.values, Matchers::Approx(original.values).margin(0.001));
  fs::remove(filename);
}
