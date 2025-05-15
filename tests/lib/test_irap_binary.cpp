#include "helpers/helper.h"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <filesystem>
#include <irap.h>
#include <irap_export.h>
#include <irap_import.h>

using namespace Catch;
using namespace surfio;
namespace fs = std::filesystem;

SCENARIO("Verify that surfio can read and write irap binary files", "[test_irap_binary.cpp]") {
  fs::path filename("surf.irap");
  auto header = irap::irap_header{
      .ncol = 6000,
      .nrow = 6000,
  };
  auto values = create_random_values(header.ncol * header.nrow);
  auto original = irap::irap{.header = header, .values = values};
  irap::to_binary_file(filename, original);
  auto imported = irap::from_binary_file(filename);

  CHECK(imported.header == original.header);
  CHECK_THAT(imported.values, Matchers::Approx(original.values).margin(0.001));
  fs::remove(filename);
}
