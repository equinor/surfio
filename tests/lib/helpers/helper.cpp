#include "helper.h"
#include <algorithm>
#include <random>

std::vector<float> create_random_values(size_t size) {
  auto values = std::vector<float>(size);

  std::mt19937 g;
  std::uniform_real_distribution<> u;
  std::generate(values.begin(), values.end(), [&]() { return u(g); });

  return values;
}
