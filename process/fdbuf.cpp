#include "process/command.hpp"
#include <cassert>
#include <errno.h>
#include <memory>
#include <streambuf>
#include <system_error>
#include <unistd.h>

namespace process {

constexpr int pipe_read = 0;
constexpr int pipe_write = 1;
constexpr std::size_t buffer_size = 4096;

fdbuf::fdbuf(int fd) : fd_(fd) {
  setg(0, 0, 0);
  setp(0, 0);
}

fdbuf::~fdbuf() { close(); }

int fdbuf::underflow() {
  assert(gptr() == egptr());

  if (fd_ < 0) {
    return traits_type::eof();
  }

  if (read_buffer_ != nullptr) {
    read_buffer_[0] = *(gptr() - 1);
  } else {
    read_buffer_ = std::make_unique<traits_type::char_type[]>(buffer_size);
  }

  const auto bytes_read = read(fd_, read_buffer_.get() + 1, buffer_size - 1);
  if (bytes_read <= 0) {
    return traits_type::eof();
  }
  setg(read_buffer_.get(), read_buffer_.get() + 1,
       read_buffer_.get() + 1 + bytes_read);
  return traits_type::to_int_type(*gptr());

  // traits_type::int_type c;

  // while (read(fd_, &c, 1) == 1)
  // {
  //     if (!traits_type::eq_int_type(c, traits_type::eof()))
  //     {
  //         buf_ = traits_type::to_char_type(c);
  //         setg(&buf_, &buf_, &buf_ + 1); // make one read position available
  //     }
  // }

  // return c;
}

fdbuf::int_type fdbuf::overflow(int_type c) {
  assert(pptr() == epptr());

  if (fd_ < 0) {
    return traits_type::eof();
  }

  if (write_buffer_ == nullptr) {
    write_buffer_ = std::make_unique<traits_type::char_type[]>(buffer_size);
  }

  if (c == traits_type::eof() || sync() == -1) {
    return traits_type::eof();
  }

  *pptr() = traits_type::to_char_type(c);
  pbump(1);
  return c;
  //     if (c != EOF)
  //     {
  //         char z = c;
  //         if (write(fd_, &z, 1) != 1)
  //         {
  //             return EOF;
  //         }
  //     }
  //     return c;
}

int fdbuf::sync() {
  if (fd_ < 0 || write_buffer_ == nullptr) {
    return 0;
  }

  char *p = pbase();
  while (p < pptr()) {
    const auto written = write(fd_, p, pptr() - p);
    if (written <= 0) {
      return -1;
    }
    p += written;
  }

  setp(write_buffer_.get(), write_buffer_.get() + buffer_size);

  return 0;
}

// std::streamsize fdbuf::xsputn(const char* s, std::streamsize num) { return
// write(fd_, s, num); }
void fdbuf::close() {
  sync();
  ::close(fd_);
}

} // namespace process
