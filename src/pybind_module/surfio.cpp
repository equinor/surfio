#include "include/irap_pybind.h"
#include "irap_export.h"
#include "irap_import.h"
#include <format>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

irap_python* make_irap_python(const irap& data) {
  constexpr auto size = sizeof(decltype(irap::values)::value_type);
  return new irap_python{
      data.header,
      {{data.header.ncol, data.header.nrow}, {size * data.header.nrow, size}, data.values.data()}
  };
}

surf_span make_surf_span(const irap_python& ip) {
  return surf_span{ip.values.data(), ip.values.shape(0), ip.values.shape(1)};
}

PYBIND11_MODULE(surfio, m) {
  py::class_<irap_header>(m, "IrapHeader")
      .def(
          py::init<
              int, double, double, double, double, double, double, int, double, double, double>(),
          py::arg("nrow"), py::arg("xori"), py::arg("xmax"), py::arg("yori"), py::arg("ymax"),
          py::arg("xinc"), py::arg("yinc"), py::arg("ncol"), py::arg("rot"), py::arg("xrot"),
          py::arg("yrot")
      )
      .def(
          "__repr__",
          [](const irap_header& header) {
            return std::format(
                "<IrapHeader(ncol={}, nrow={}, xory={}, yori={}, xinc={}, yinc={}, xmax={}, "
                "ymax={}, rot={}, xrot={}, yrot={})>",
                header.ncol, header.nrow, header.xori, header.yori, header.xinc, header.yinc,
                header.xmax, header.ymax, header.rot, header.xrot, header.yrot
            );
          }
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
      .def_readwrite("ncol", &irap_header::ncol)
      .def_readwrite("nrow", &irap_header::nrow);

  py::class_<irap_python>(m, "IrapSurface")
      .def(py::init<irap_header, py::array_t<float>>(), py::arg("header"), py::arg("values"))
      .def(
          "__repr__",
          [](const irap_python& ip) {
            return std::format(
                "<IrapSurface(header=IrapHeader(ncol={}, nrow={}, xory={}, yori={}, xinc={}, "
                "yinc={}, xmax={}, ymax={}, rot={}, xrot={}, yrot={}), values=...)>",
                ip.header.ncol, ip.header.nrow, ip.header.xori, ip.header.yori, ip.header.xinc,
                ip.header.yinc, ip.header.xmax, ip.header.ymax, ip.header.rot, ip.header.xrot,
                ip.header.yrot
            );
          }
      )
      .def_readwrite("header", &irap_python::header)
      .def_readwrite("values", &irap_python::values)
      .def_static(
          "import_ascii_file",
          [](const std::string& path) -> irap_python* {
            auto irap = import_irap_ascii(path);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def_static(
          "import_ascii",
          [](const std::string& string) -> irap_python* {
            auto irap = import_irap_ascii_from_string(string);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def_static(
          "import_binary_file",
          [](const std::string& path) -> irap_python* {
            auto irap = import_irap_binary(path);
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def_static(
          "import_binary",
          [](const py::bytes& buffer) -> irap_python* {
            auto irap = import_irap_binary_from_buffer(std::string_view(buffer));
            // lock the GIL before creating the numpy array
            py::gil_scoped_acquire acquire;
            return make_irap_python(irap);
          },
          py::call_guard<py::gil_scoped_release>()
      )
      .def(
          "export_ascii",
          [](const irap_python& ip) -> std::string {
            return export_irap_to_ascii_string(ip.header, make_surf_span(ip));
          }
      )
      .def(
          "export_ascii_file",
          [](const irap_python& ip, const std::string& filename) -> void {
            export_irap_to_ascii_file(filename, ip.header, make_surf_span(ip));
          }
      )
      .def(
          "export_binary",
          [](const irap_python& ip) -> py::bytes {
            auto buffer = export_irap_to_binary_string(ip.header, make_surf_span(ip));
            return py::bytes(buffer);
          }
      )
      .def("export_binary_file", [](const irap_python& ip, const std::string& filename) -> void {
        export_irap_to_binary_file(filename, ip.header, make_surf_span(ip));
      });
}
