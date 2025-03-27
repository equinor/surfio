#include <memory>
#include <string>

struct internals;

class mmap_file {
public:
  mmap_file(const std::string& filename);
  ~mmap_file();
  const char* begin() const;
  const char* end() const;

private:
  std::unique_ptr<internals> d;
};
