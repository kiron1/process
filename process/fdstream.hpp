#pragma once

#include "process/fdbuf.hpp"
#include <ostream>

namespace process {

/// Output stream of a POSIX file descriptor.
class fdostream : public std::ostream {
public:
  explicit fdostream(int fd) : std::ostream(0), buf(fd) { rdbuf(&buf); }
  void close() { buf.close(); }

private:
  fdbuf buf;
};

/// Input stream of a POSIX file descriptor.
class fdistream : public std::istream {
public:
  explicit fdistream(int fd) : std::istream(0), buf(fd) { rdbuf(&buf); }
  void close() { buf.close(); }

private:
  fdbuf buf;
};

} // namespace process
