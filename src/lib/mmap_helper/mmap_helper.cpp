#include "mmap_helper.h"
#include "llfio/llfio.hpp"
#include <format>
#include <stdexcept>

namespace llfio = LLFIO_V2_NAMESPACE;
struct internals {
  llfio::mapped_file_handle handle;
  const char* end;
};

mmap_file::mmap_file(const std::string& filename) {
  auto result = llfio::mapped_file({}, filename);
  if (!result) {
    throw std::runtime_error(
        std::format(
            "failed to map file :{}, with error: {}", filename,
            result.error().message()
        )
    );
  }
  auto& file_handle = result.value();

  auto length = file_handle.maximum_extent().value();
  if (length == 0)
    throw std::length_error(std::format("file is empty: {}", filename));

  d = std::make_unique<internals>(
      std::move(file_handle),
      reinterpret_cast<const char*>(file_handle.address() + length)
  );
}

mmap_file::~mmap_file() { std::ignore = d->handle.close(); }
const char* mmap_file::begin() const {
  return reinterpret_cast<const char*>(d->handle.address());
}
const char* mmap_file::end() const { return d->end; }
