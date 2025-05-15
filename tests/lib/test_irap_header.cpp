#include "irap.h"
#include <catch2/catch_test_macros.hpp>
#include <pybind11/embed.h>

namespace py = pybind11;
using namespace py::literals;
using namespace surfio;

TEST_CASE("Check that python and C++ default initialize IrapHeader the same way") {
  irap::irap_header cpp_header{.ncol = 200, .nrow = 200};
  py::scoped_interpreter guard{};
  py::module_ surfio = py::module_::import("surfio");
  auto python_header = surfio.attr("IrapHeader")("ncol"_a = 200, "nrow"_a = 200);
  auto python_header_cpp = python_header.cast<irap::irap_header>();
  CHECK(cpp_header.ncol == python_header_cpp.ncol);
  CHECK(cpp_header.nrow == python_header_cpp.nrow);
  CHECK(cpp_header.xori == python_header_cpp.xori);
  CHECK(cpp_header.yori == python_header_cpp.yori);
  CHECK(cpp_header.xmax == python_header_cpp.xmax);
  CHECK(cpp_header.ymax == python_header_cpp.ymax);
  CHECK(cpp_header.xinc == python_header_cpp.xinc);
  CHECK(cpp_header.yinc == python_header_cpp.yinc);
  CHECK(cpp_header.rot == python_header_cpp.rot);
  CHECK(cpp_header.xrot == python_header_cpp.xrot);
  CHECK(cpp_header.yrot == python_header_cpp.yrot);
}
