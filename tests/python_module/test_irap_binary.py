import struct
from io import BytesIO
import sys

import numpy as np
import pytest
import surfio
import xtgeo


def compare_xtgeo_surface_with_surfio_header(
    xtgeo_surface: xtgeo.RegularSurface, surfio_header: surfio.IrapHeader
):
    assert xtgeo_surface.ncol == surfio_header.ncol
    assert xtgeo_surface.nrow == surfio_header.nrow
    assert xtgeo_surface.xori == surfio_header.xori
    assert xtgeo_surface.yori == surfio_header.yori
    assert xtgeo_surface.xinc == surfio_header.xinc
    assert xtgeo_surface.yinc == surfio_header.yinc
    assert xtgeo_surface.xmax == surfio_header.xmax
    assert xtgeo_surface.ymax == surfio_header.ymax
    assert xtgeo_surface.rotation == surfio_header.rot


def test_reading_empty_file_errors(tmp_path):
    irap_path = tmp_path / "test.irap"
    irap_path.write_text("")
    with pytest.raises(RuntimeError, match="failed to map file"):
        surfio.IrapSurface.from_ascii_file(str(irap_path))


def test_reading_short_header_results_in_value_error():
    with pytest.raises(ValueError, match="Failed to read irap headers"):
        _ = surfio.IrapSurface.from_ascii_string("-996 1")


def test_short_files_result_in_value_error():
    values = np.random.normal(2000, 50, size=12)
    srf = xtgeo.RegularSurface(ncol=3, nrow=4, xinc=1, yinc=1, values=values)
    file_buffer = BytesIO()
    srf.to_file(file_buffer, "irap_binary")
    file_buffer.truncate(100)
    with pytest.raises(ValueError, match="ncol and nrow declared in header"):
        _ = surfio.IrapSurface.from_binary_buffer(file_buffer.getvalue())


def test_binary_xtgeo_is_imported_correctly_in_surfio():
    values = np.random.normal(2000, 50, size=12)
    srf = xtgeo.RegularSurface(ncol=3, nrow=4, xinc=1, yinc=1, values=values)
    file_buffer = BytesIO()
    srf.to_file(file_buffer, "irap_binary")
    srf_imported = surfio.IrapSurface.from_binary_buffer(file_buffer.getvalue())
    assert np.allclose(srf.values, srf_imported.values)
    compare_xtgeo_surface_with_surfio_header(srf, srf_imported.header)


def test_surfio_can_import_data_exported_from_surfio():
    srf = surfio.IrapSurface(
        surfio.IrapHeader(ncol=3, nrow=2, xinc=1.0, yinc=1.0, xmax=2.0, ymax=1.0),
        values=np.arange(6, dtype=np.float32).reshape((3, 2)),
    )
    buffer = srf.to_binary_buffer()
    srf_imported = surfio.IrapSurface.from_binary_buffer(buffer)

    assert np.allclose(srf.values, srf_imported.values)
    assert srf.header == srf_imported.header


def test_surfio_can_export_values_in_fortran_order():
    srf = surfio.IrapSurface(
        surfio.IrapHeader(ncol=3, nrow=2, xinc=1.0, yinc=1.0, xmax=2.0, ymax=1.0),
        values=np.asfortranarray(np.arange(6, dtype=np.float32).reshape((3, 2))),
    )
    buffer = srf.to_binary_buffer()
    srf_imported = surfio.IrapSurface.from_binary_buffer(buffer)

    assert np.allclose(srf.values, srf_imported.values)
    assert srf.header == srf_imported.header


def test_xtgeo_can_import_data_exported_from_surfio(tmp_path):
    srf = surfio.IrapSurface(
        surfio.IrapHeader(ncol=3, nrow=2, xinc=1.0, yinc=1.0, xmax=2.0, ymax=1.0),
        values=np.arange(6, dtype=np.float32).reshape((3, 2)),
    )
    srf.to_binary_file(str(tmp_path / "test.irap"))
    srf_imported = xtgeo.surface_from_file(tmp_path / "test.irap")

    assert np.allclose(srf.values, srf_imported.values)
    compare_xtgeo_surface_with_surfio_header(srf_imported, srf.header)


def test_exporting_nan() -> None:
    surface = surfio.IrapSurface(
        surfio.IrapHeader(ncol=1, nrow=1, xinc=2.0, yinc=2.0, xmax=2.0, ymax=2.0),
        values=np.array([[np.nan]], dtype=np.float32),
    )

    srf_export = surface.to_binary_buffer()
    assert struct.unpack("f", srf_export[107:103:-1])[0] >= 1e30


@pytest.mark.skipif(sys.platform == "win32", reason="memray not available on windows")
@pytest.mark.limit_memory("10 MB")
def test_that_header_declarations_do_not_cause_increased_allocation() -> None:
    # Surface has just one value, but header declares 2**22 values ~64mb
    surface = surfio.IrapSurface(
        surfio.IrapHeader(
            ncol=2**12, nrow=2**12, xinc=2.0, yinc=2.0, xmax=2.0, ymax=2.0
        ),
        values=np.array([[0.0]], dtype=np.float32),
    )

    buf = surface.to_binary_buffer()
    with pytest.raises(ValueError, match="ncol and nrow declared in header"):
        surfio.IrapSurface.from_binary_buffer(buf)


def test_that_negative_ncol_and_nrow_results_in_value_error():
    with pytest.raises(ValueError, match="negative nrow"):
        # The following is a surface that can be recreated with
        #
        # IrapSurface(
        #     IrapHeader(ncol=-1, nrow=-2, xinc=1.0, yinc=1.0, xmax=2.0, ymax=1.0),
        #     values=np.arange(2, dtype=np.float32).reshape((1, 2)),
        # ).to_binary_buffer()
        # Including it here as bytes to avoid having to deal with validation on
        # before calling to_binary_buffer
        surfio.IrapSurface.from_binary_buffer(
            b"\x00\x00\x00 \xff\xff\xfc\x1c\xff\xff\xff\xfe\x00\x00\x00\x00\xc0"
            b"\x00\x00\x00\x00\x00\x00\x00\xc0@\x00\x00?\x80\x00\x00?\x80\x00"
            b"\x00\x00\x00\x00 \x00\x00\x00\x10\xff\xff\xff\xff\x00\x00\x00\x00"
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x1c"
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1c"
            b"\x00\x00\x00\x08\x00\x00\x00\x00?\x80\x00\x00\x00\x00\x00\x08"
        )
