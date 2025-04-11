import struct
from io import BytesIO

import numpy as np
import pytest
import surfio
import xtgeo


def test_reading_empty_file_errors(tmp_path):
    irap_path = tmp_path / "test.irap"
    irap_path.write_text("")
    with pytest.raises(ValueError, match="file is empty"):
        surfio.IrapSurface.import_ascii_file(str(irap_path))


def test_reading_short_header_results_in_value_error():
    with pytest.raises(ValueError, match="Failed to read irap headers"):
        _ = surfio.IrapSurface.import_ascii("-996 1")


def test_short_files_result_in_value_error():
    values = np.random.normal(2000, 50, size=12)
    srf = xtgeo.RegularSurface(ncol=3, nrow=4, xinc=1, yinc=1, values=values)
    file_buffer = BytesIO()
    srf.to_file(file_buffer, "irap_binary")
    file_buffer.truncate(100)
    with pytest.raises(ValueError, match="End of file reached"):
        _ = surfio.IrapSurface.import_binary(file_buffer.getvalue())


def test_binary_xtgeo_is_imported_correctly_in_surfio():
    values = np.random.normal(2000, 50, size=12)
    srf = xtgeo.RegularSurface(ncol=3, nrow=4, xinc=1, yinc=1, values=values)
    file_buffer = BytesIO()
    srf.to_file(file_buffer, "irap_binary")
    srf_imported = surfio.IrapSurface.import_binary(file_buffer.getvalue())
    assert np.allclose(srf.values, srf_imported.values)


def test_surfio_can_import_data_exported_from_surfio():
    srf = surfio.IrapSurface(
        surfio.IrapHeader(
            nx=3,
            ny=2,
            xori=0.0,
            yori=0.0,
            xinc=1.0,
            yinc=1.0,
            xmax=8.0,
            ymax=8.0,
            rot=0.0,
            xrot=0.0,
            yrot=0.0,
        ),
        values=np.arange(6, dtype=np.float32).reshape((3, 2)),
    )
    buffer = srf.export_binary()
    srf_imported = surfio.IrapSurface.import_binary(buffer)

    assert np.allclose(srf.values, srf_imported.values)


def test_xtgeo_can_import_data_exported_from_surfio(tmp_path):
    srf = surfio.IrapSurface(
        surfio.IrapHeader(
            nx=3,
            ny=2,
            xori=0.0,
            yori=0.0,
            xinc=1.0,
            yinc=1.0,
            xmax=8.0,
            ymax=8.0,
            rot=0.0,
            xrot=0.0,
            yrot=0.0,
        ),
        values=np.arange(6, dtype=np.float32).reshape((3, 2)),
    )
    srf.export_binary_file(str(tmp_path / "test.irap"))
    srf_imported = xtgeo.surface_from_file(tmp_path / "test.irap")

    assert np.allclose(srf.values, srf_imported.values)


def test_exporting_nan_maps_to_9999900():
    surface = surfio.IrapSurface(
        surfio.IrapHeader(
            nx=1,
            ny=1,
            xori=0.0,
            yori=0.0,
            xinc=2.0,
            yinc=2.0,
            xmax=2.0,
            ymax=2.0,
            rot=0.0,
            xrot=0.0,
            yrot=0.0,
        ),
        values=np.array([[np.nan]], dtype=np.float32),
    )

    srf_export = surface.export_binary()
    assert struct.unpack("f", srf_export[107:103:-1]) == (9999900.0,)
