cmake_minimum_required(VERSION 3.22.0)
project(surfio LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if(MSVC)
  add_compile_options(/W4 /WX /sdl /permissive-)
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Gy /GL")
else()
  add_compile_options(-Wall -Wextra -Werror)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# dependencies
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11
  GIT_TAG a2e59f0e7065404b44dfe92a28aca47ba1378dc4) # v2.13
FetchContent_MakeAvailable(pybind11)

# mdspan is part of C++23. kokkos/mdspan is the reference implementation for the
# standard proposal. This can be removed when all needed compilers/libs support
# it.
FetchContent_Declare(
  mdspan
  GIT_REPOSITORY https://github.com/kokkos/mdspan.git
  GIT_TAG 9ceface91483775a6c74d06ebf717bbb2768452f) # mdspan-0.6.0
FetchContent_MakeAvailable(mdspan)

FetchContent_Declare(
  mio
  GIT_REPOSITORY https://github.com/vimpunk/mio
  GIT_TAG 8b6b7d878c89e81614d05edca7936de41ccdd2da)
FetchContent_MakeAvailable(mio)

find_package(Python 3.11 REQUIRED COMPONENTS Development.Module Interpreter
                                             NumPy)
cmake_policy(SET CMP0167 NEW)

# surfio_lib
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/src/lib")
add_library(surfio_lib STATIC)
target_sources(
  surfio_lib
  PRIVATE ${SRC_PATH}/mmap_wrapper/mmap_wrapper.cpp
          ${SRC_PATH}/irap_import_ascii.cpp ${SRC_PATH}/irap_import_binary.cpp
          ${SRC_PATH}/irap_export_ascii.cpp ${SRC_PATH}/irap_export_binary.cpp)
target_link_libraries(surfio_lib INTERFACE mdspan)
target_include_directories(
  surfio_lib
  INTERFACE ${SRC_PATH}/include
  PUBLIC $<BUILD_INTERFACE:${mdspan_SOURCE_DIR}/include/>
  PRIVATE ${mio_SOURCE_DIR}/single_include/mio)

# surfio python module
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/src/pybind_module")
pybind11_add_module(surfio MODULE ${SRC_PATH}/surfio.cpp)
target_link_libraries(surfio PRIVATE surfio_lib)
