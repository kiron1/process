#pragma once

#include "process/fdstream.hpp"
#include <memory>
#include <string>
#include <vector>

namespace process {

/// Represent a command which can be executed.
class command {
public:
  class handle {
    struct impl;
    friend class ::process::command;

  public:
    int join();

    fdostream &stdin();
    fdistream &stdout();

  private:
    std::shared_ptr<impl> impl_;
    handle(int pid, int stdin, int stdout);
  };

  command(std::string command) : command_(std::move(command)) {}

  std::string program_path() const;

  command &arg(std::string a) {
    args_.push_back(std::move(a));
    return *this;
  }

  handle run();

private:
  std::string command_;
  std::vector<std::string> args_;
};

} // namespace process
