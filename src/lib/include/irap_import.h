#pragma once

#include "irap.h"
#include <span>
#include <string>

irap import_irap_ascii(std::string path);
irap import_irap_ascii_from_string(const std::string& buffer);
irap import_irap_binary(std::string path);
irap import_irap_binary_from_buffer(std::span<const char> buffer);
