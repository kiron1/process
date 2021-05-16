#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <streambuf>
#include <string>
#include <vector>

namespace process {

/// \c std::streambuf arround a POSIX file descriptor.
class fdbuf : public std::streambuf {
public:
  fdbuf(int fd);
  ~fdbuf();
  void close();

protected:
  int underflow();
  virtual int_type overflow(int_type c);
  int sync();

private:
  int fd_ = -1; // file descriptor
  std::unique_ptr<char_type[]> read_buffer_ = nullptr;
  std::unique_ptr<char_type[]> write_buffer_ = nullptr;
};

} // namespace process
