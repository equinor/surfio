import statistics
import timeit
from tempfile import TemporaryDirectory

import numpy as np
import xtgeo


def _print_result(lib_name, timings):
    print(f"{lib_name} ran {len(timings)} times:")
    print(f"    {sum(timings)}s")
    print(
        f"    Fastest: {min(timings):.4f}s, "
        f"slowest {max(timings):.4f}s, "
        f"mean {statistics.mean(timings):.4f}s"
    )


def print_results(xtgeo_timings, surfio_timings):
    _print_result("xtgeo", xtgeo_timings)
    _print_result("surfio", surfio_timings)

    percentage_improvement = sum(xtgeo_timings) / sum(surfio_timings)
    print(f"Surfio was {percentage_improvement:.2f} times faster")


with TemporaryDirectory() as dir:
    values = np.random.normal(0, 500, size=16000000)
    srf = xtgeo.RegularSurface(ncol=4000, nrow=4000, xinc=1, yinc=1, values=values)
    input_file_binary = f"{dir}/input.irap"
    input_file_ascii = f"{dir}/input.irap_ascii"
    output_file = f"{dir}/output.irap"
    srf.to_file(f"{input_file_binary}", "irap_binary")
    srf.to_file(f"{input_file_ascii}", "irap_ascii")

    print("Comparing importing and exporting irap files")
    print("ncol=4000, nrow=4000, random values")

    print("\n\nExporting binary irap files")
    xtgeo_result = timeit.repeat(
        setup=f'import xtgeo; surf = xtgeo.surface_from_file("{input_file_binary}", fformat="irap_binary")',
        stmt=f'surf.to_file("{output_file}", fformat="irap_binary")',
        number=1,
        repeat=10,
    )
    surfio_result = timeit.repeat(
        setup=f'from surfio import IrapSurface; surf = IrapSurface.import_binary_file("{input_file_binary}")',
        stmt=f'IrapSurface.export_binary_file(surf, "{output_file}")',
        number=1,
        repeat=10,
    )
    print_results(xtgeo_result, surfio_result)

    print("\n\nExporting ascii irap files")
    xtgeo_result = timeit.repeat(
        setup=f'import xtgeo; surf = xtgeo.surface_from_file("{input_file_binary}", fformat="irap_binary")',
        stmt=f'surf.to_file("{output_file}", fformat="irap_ascii")',
        number=1,
        repeat=10,
    )
    surfio_result = timeit.repeat(
        setup=f'from surfio import IrapSurface; surf = IrapSurface.import_binary_file("{input_file_binary}")',
        stmt=f'IrapSurface.export_binary_file(surf, "{output_file}")',
        number=1,
        repeat=10,
    )
    print_results(xtgeo_result, surfio_result)

    print("\n\nImporting binary irap files")
    xtgeo_result = timeit.repeat(
        setup="import xtgeo",
        stmt=f'xtgeo.surface_from_file("{input_file_binary}", fformat="irap_binary")',
        number=1,
        repeat=10,
    )
    surfio_result = timeit.repeat(
        setup="from surfio import IrapSurface",
        stmt=f'IrapSurface.import_binary_file("{input_file_binary}")',
        number=1,
        repeat=10,
    )
    print_results(xtgeo_result, surfio_result)

    print("\n\nImporting ascii irap files")
    xtgeo_result = timeit.repeat(
        setup="import xtgeo",
        stmt=f'xtgeo.surface_from_file("{input_file_ascii}", fformat="irap_ascii")',
        number=1,
        repeat=10,
    )
    surfio_result = timeit.repeat(
        setup="from surfio import IrapSurface",
        stmt=f'IrapSurface.import_ascii_file("{input_file_ascii}")',
        number=1,
        repeat=10,
    )
    print_results(xtgeo_result, surfio_result)
