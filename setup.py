import os
import subprocess
import sys
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
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve()

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        # Create a conan profile if none exist
        subprocess.run(["conan", "profile", "detect", "-e"])

        subprocess.run(
            [
                "conan",
                "install",
                f"{ext.sourcedir}",
                "--build=missing",
                "-s",
                "compiler.cppstd=20",
                "-s",
                f"build_type={cfg}",
                f"--output-folder={extdir}",
                "-c",
                "tools.cmake.cmake_layout:build_folder_vars=['const.wheel', 'options.build_type']",
            ],
            check=True,
        )

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}",
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}{os.sep}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
        ]
        build_args = []
        # Adding CMake arguments set as environment variable
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        subprocess.run(
            [
                "cmake",
                f"--preset conan-wheel{'' if sys.platform.startswith('win') else '-' + cfg.lower()}",
                *cmake_args,
            ],
            check=True,
        )
        subprocess.run(
            [
                "cmake",
                "--build",
                f"--preset conan-wheel-{cfg.lower()}",
                *build_args,
            ],
            check=True,
        )


setup(
    name="surfio",
    ext_modules=[CMakeExtension("surfio")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
)
