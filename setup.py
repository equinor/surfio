import os
import platform
import shutil
import subprocess
import sys
from contextlib import suppress
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "debug" if debug else "release"
        build_dir = f"build/wheel/${cfg}"

        # Delete the old build directory
        with suppress(Exception):
            shutil.rmtree(build_dir)

        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}{os.sep}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
        ]

        # Custom preset for windows that lets us use the ninja generator
        preset = cfg + ("-windows" if platform.system().startswith("Win") else "-posix")

        # Adding CMake arguments set as environment variable
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        subprocess.run(
            [
                "cmake",
                "--preset",
                preset,
                "-B",
                build_dir,
                *cmake_args,
            ],
            check=True,
        )
        subprocess.run(
            ["cmake", "--build", build_dir, "--parallel"],
            check=True,
        )


setup(
    name="surfio",
    ext_modules=[CMakeExtension("surfio")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
)
