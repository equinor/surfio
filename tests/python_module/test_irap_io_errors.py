from pathlib import Path

import numpy as np
import pytest
import surfio


@pytest.fixture(params=["to_ascii_file", "to_binary_file"])
def to_file(request):
    surface = surfio.IrapSurface(
        surfio.IrapHeader(ncol=1, nrow=1), values=np.zeros((1, 1), dtype=np.float32)
    )
    return getattr(surface, request.param)


def test_that_writing_to_a_missing_directory_results_in_file_not_found_error(
    to_file, tmp_path
):
    with pytest.raises(FileNotFoundError, match=r"surface\.irap"):
        to_file(tmp_path / "missing" / "surface.irap")


@pytest.mark.skipif(not Path("/dev/full").exists(), reason="requires /dev/full")
def test_that_writing_to_a_full_device_results_in_os_error(to_file):
    with pytest.raises(OSError, match="No space left on device"):
        to_file(Path("/dev/full"))
