cmake_minimum_required(VERSION 3.22.0)
project(surfio LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# dependencies
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11
  GIT_TAG a2e59f0e7065404b44dfe92a28aca47ba1378dc4 # v2.13
)
FetchContent_MakeAvailable(pybind11)

# mdspan is part of C++23. kokkos/mdspan is the reference implementation for the standard proposal.
# This can be removed when all needed compilers/libs support it.
FetchContent_Declare(
  mdspan
  GIT_REPOSITORY https://github.com/kokkos/mdspan.git
  GIT_TAG 9ceface91483775a6c74d06ebf717bbb2768452f # mdspan-0.6.0
)
FetchContent_MakeAvailable(mdspan)

FetchContent_Declare(
  mio
  GIT_REPOSITORY https://github.com/vimpunk/mio
  GIT_TAG 8b6b7d878c89e81614d05edca7936de41ccdd2da
)
FetchContent_MakeAvailable(mio)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG 2b60af89e23d28eefc081bc930831ee9d45ea58b # v3.8.1
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

find_package(Python 3.11 REQUIRED COMPONENTS Development.Module Interpreter NumPy)
cmake_policy(SET CMP0167 NEW)

# compile options as a target
add_library(surfio-compile-options INTERFACE)
if(MSVC)
  target_compile_options(surfio-compile-options INTERFACE /W4 /WX /sdl /permissive-)
  target_compile_options(surfio-compile-options INTERFACE "$<$<CONFIG:Release>:/O2 /Oi /Gy>")
else()
  target_compile_options(surfio-compile-options INTERFACE -Wall -Wextra -Werror)
  target_compile_options(surfio-compile-options INTERFACE "$<$<CONFIG:Release>:-O3>")
endif()

# surfio_lib
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/src/lib")
add_library(surfio_lib STATIC)
target_sources(
  surfio_lib
  PRIVATE ${SRC_PATH}/mmap_wrapper/mmap_wrapper.cpp ${SRC_PATH}/irap_import_ascii.cpp
          ${SRC_PATH}/irap_import_binary.cpp ${SRC_PATH}/irap_export_ascii.cpp
          ${SRC_PATH}/irap_export_binary.cpp
)
target_link_libraries(
  surfio_lib
  INTERFACE mdspan
  PRIVATE surfio-compile-options
)
target_include_directories(
  surfio_lib
  INTERFACE ${SRC_PATH}/include
  PUBLIC $<BUILD_INTERFACE:${mdspan_SOURCE_DIR}/include/>
  PRIVATE ${mio_SOURCE_DIR}/single_include/mio
)

# surfio python module
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/src/pybind_module")
pybind11_add_module(surfio MODULE ${SRC_PATH}/surfio.cpp)
target_link_libraries(surfio PRIVATE surfio_lib surfio-compile-options)

# C++ tests
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/tests/lib")
add_executable(tests ${SRC_PATH}/test_irap_ascii.cpp ${SRC_PATH}/test_irap_binary.cpp)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain surfio_lib)
include(CTest)
include(Catch)
catch_discover_tests(tests)
