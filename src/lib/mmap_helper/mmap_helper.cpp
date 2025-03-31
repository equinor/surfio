#include "mmap_helper.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include <filesystem>
#include <format>
#include <stdexcept>

using namespace boost::iostreams;
struct internals {
  mapped_file_source handle;
};

namespace fs = std::filesystem;

mmap_file::mmap_file(const std::string& filename) {
  if (!fs::exists(filename))
    throw std::invalid_argument(std::format("file not found: {}", filename));

  if (!fs::file_size(filename))
    throw std::length_error(std::format("file is empty: {}", filename));

  auto file = mapped_file_source(filename);
  if (!file.is_open()) {
    throw std::runtime_error(std::format("failed to map file :{}", filename));
  }

  d = std::make_unique<internals>(std::move(file));
}

mmap_file::~mmap_file() {}
const char* mmap_file::begin() const { return d->handle.begin(); }
const char* mmap_file::end() const { return d->handle.end(); }
