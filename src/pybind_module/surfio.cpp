#include "irap_import.h"
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <sstream>
namespace py = pybind11;

struct irap_python {
  irap_header header;
  py::array_t<float> values;
};

static void write_header(irap_header header, std::ostream& out) {
  out << std::setprecision(6) << std::fixed << std::showpoint;
  out << "-996 " << header.ny << " " << header.xinc << " " << header.yinc
      << "\n";
  out << header.xori << " " << header.xmax << " " << header.yori << " "
      << header.ymax << "\n";
  out << header.nx << " " << header.rot << " " << header.xrot << " "
      << header.yrot << "\n";
  out << "0  0  0  0  0  0  0\n";
}

constexpr int MAX_PER_LINE = 9; // Maximum accepted by some software

static void write_values(py::array_t<float> values, std::ostream& out) {
  auto vs = values.unchecked<2>();
  int written_on_line = 0;
  out << std::setprecision(4) << std::fixed << std::showpoint;
  for (py::ssize_t i = 0; i < vs.shape(1); i++) {
    for (py::ssize_t j = 0; j < vs.shape(0); j++) {
      auto v = vs(j, i);
      if (std::isnan(v))
        out << UNDEF_MAP_IRAP;
      else
        out << vs(j, i);
      if (++written_on_line < MAX_PER_LINE) {
        out << " ";
      } else {
        out << "\n";
        written_on_line = 0;
      }
    }
  }
}

PYBIND11_MODULE(surfio, m) {
  py::class_<irap_header>(m, "IrapHeader")
      .def(
          py::init<
              int, double, double, double, double, double, double, int, double,
              double, double>(),
          py::arg("ny"), py::arg("xori"), py::arg("xmax"), py::arg("yori"),
          py::arg("ymax"), py::arg("xinc"), py::arg("yinc"), py::arg("nx"),
          py::arg("rot"), py::arg("xrot"), py::arg("yrot")
      )
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def_readonly_static("id", &irap_header::id)
      .def_readwrite("rot", &irap_header::rot)
      .def_readwrite("xinc", &irap_header::xinc)
      .def_readwrite("yinc", &irap_header::yinc)
      .def_readwrite("xori", &irap_header::xori)
      .def_readwrite("yori", &irap_header::yori)
      .def_readwrite("xmax", &irap_header::xmax)
      .def_readwrite("ymax", &irap_header::ymax)
      .def_readwrite("xrot", &irap_header::xrot)
      .def_readwrite("yrot", &irap_header::yrot)
      .def_readwrite("nx", &irap_header::nx)
      .def_readwrite("ny", &irap_header::ny);

  py::class_<irap_python>(m, "IrapSurface")
      .def(
          py::init<irap_header, py::array_t<float>>(), py::arg("header"),
          py::arg("values")
      )
      .def(
          "__repr__",
          [](const irap_python& ip) {
            return std::format(
                "<IrapSurface(nx={}, ny={}, xory={}, yori={}, "
                "xinc={}, yinc={}, xmax={}, ymax={}, rot={}, "
                "xrot={}, yrot={})>",
                ip.header.nx, ip.header.ny, ip.header.xori, ip.header.yori,
                ip.header.xinc, ip.header.yinc, ip.header.xmax, ip.header.ymax,
                ip.header.rot, ip.header.xrot, ip.header.yrot
            );
          }
      )
      .def_readwrite("header", &irap_python::header)
      .def_readwrite("values", &irap_python::values)
      .def_static(
          "import_ascii_file",
          [](const std::string& path) {
            auto irap = import_irap_ascii(path);
            constexpr auto size = sizeof(decltype(irap.values)::value_type);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return new irap_python{
                irap.header,
                {{irap.header.nx, irap.header.ny},
                 {size * irap.header.ny, size},
                 irap.values.data()}
            };
          },
          py::call_guard<py::gil_scoped_release>(),
          py::return_value_policy::take_ownership
      )
      .def_static(
          "import_ascii",
          [](const std::string& string) {
            auto irap = import_irap_ascii_string(string);
            constexpr auto size = sizeof(decltype(irap.values)::value_type);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return new irap_python{
                irap.header,
                {{irap.header.nx, irap.header.ny},
                 {size * irap.header.ny, size},
                 irap.values.data()}
            };
          },
          py::call_guard<py::gil_scoped_release>(),
          py::return_value_policy::take_ownership
      )
      .def(
          "export_ascii",
          [](const irap_python& ip) {
            std::ostringstream out;
            write_header(ip.header, out);
            write_values(ip.values, out);
            return out.str();
          }
      )
      .def(
          "export_ascii_file",
          [](const irap_python& ip, const std::string& filename) {
            std::ofstream out(filename);
            write_header(ip.header, out);
            write_values(ip.values, out);
          }
      );
}
