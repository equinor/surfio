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
  GIT_TAG v2.13)
FetchContent_MakeAvailable(pybind11)

find_package(Python 3.9 REQUIRED COMPONENTS Development.Module Interpreter
                                            NumPy)
cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED COMPONENTS iostreams)

# surfio_lib
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/src/lib")
add_library(surfio_lib STATIC)
target_sources(surfio_lib PRIVATE ${SRC_PATH}/mmap_helper/mmap_helper.cpp
                                  ${SRC_PATH}/irap_import.cpp)
target_include_directories(surfio_lib INTERFACE ${SRC_PATH}/include)
target_link_libraries(surfio_lib PRIVATE Boost::iostreams)

# surfio python module
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/src/pybind_module")
pybind11_add_module(surfio MODULE ${SRC_PATH}/surfio.cpp)
target_link_libraries(surfio PRIVATE surfio_lib)
