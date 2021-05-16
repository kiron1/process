#include "process/command.hpp"
#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string_view>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

extern char **environ;

namespace process {

constexpr int pipe_read = 0;
constexpr int pipe_write = 1;

struct command::handle::impl {
  impl(int pid, int stdin, int stdout)
      : pid(pid), stdin(stdin), stdout(stdout) {}
  int pid;
  fdostream stdin;
  fdistream stdout;
};

command::handle::handle(int pid, int stdinfd, int stdoutfd)
    : impl_(std::make_unique<impl>(pid, stdinfd, stdoutfd)) {}

int command::handle::join() {
  int wpid;
  int status = 0;
  do {
    wpid = wait(&status);
  } while (wpid != impl_->pid && !WIFEXITED(status));
  stdin().close();
  stdout().close();
  return WEXITSTATUS(status);
}

fdostream &command::handle::stdin() { return impl_->stdin; }
fdistream &command::handle::stdout() { return impl_->stdout; }

std::string command::program_path() const {

  auto first = std::size_t{0};
  auto path = std::string_view(getenv("PATH"));

  while (true) {
    const auto last = path.find(':', first);
    const auto last_pos = (last == std::string_view::npos) ? path.size() : last;
    const auto len = last_pos - first;
    const auto prefix = path.substr(first, len);
    const auto path_name = std::string(prefix) + "/" + command_;

    if (access(path_name.c_str(), R_OK | X_OK) == 0) {
      return path_name;
    }
    if (last == std::string_view::npos) {
      break;
    }
    first = last + 1;
  }
  return "";
}

command::handle command::run() {
  int stdin_pipe[2];
  int stdout_pipe[2];

  if (pipe(stdin_pipe) < 0) {
    throw std::system_error(errno, std::generic_category(),
                            "allocating pipe for child input redirect");
  }
  if (pipe(stdout_pipe) < 0) {
    close(stdin_pipe[pipe_read]);
    close(stdin_pipe[pipe_write]);
    throw std::system_error(errno, std::generic_category(),
                            "allocating pipe for child output redirect");
  }

  const auto child_pid = fork();
  if (0 == child_pid) {
    // child continues here

    // redirect stdin
    if (dup2(stdin_pipe[pipe_read], STDIN_FILENO) == -1) {
      exit(errno);
    }

    // redirect stdout
    if (dup2(stdout_pipe[pipe_write], STDOUT_FILENO) == -1) {
      exit(errno);
    }

    // redirect stderr
    if (dup2(stdout_pipe[pipe_write], STDERR_FILENO) == -1) {
      exit(errno);
    }

    // all these are for use by parent only
    close(stdin_pipe[pipe_read]);
    close(stdin_pipe[pipe_write]);
    close(stdout_pipe[pipe_read]);
    close(stdout_pipe[pipe_write]);

    // run child process image
    // replace this with any exec* function find easier to use ("man exec")
    auto args = std::vector<char *>{};
    args.reserve(1 + args_.size() + 1);
    args.push_back(&*command_.begin());
    for (auto &arg : args_) {
      args.push_back(&*arg.begin());
    }
    args.push_back(nullptr);
    const auto execrc = execve(command_.c_str(), args.data(), environ);
    // if we get here at all, an error occurred, but we are in the child
    // process, so just exit
    exit(execrc);
  } else if (child_pid > 0) {
    // parent continues here

    // close unused file descriptors, these are for child only
    close(stdin_pipe[pipe_read]);
    close(stdout_pipe[pipe_write]);
  } else {
    // failed to create child
    close(stdin_pipe[pipe_read]);
    close(stdin_pipe[pipe_write]);
    close(stdout_pipe[pipe_read]);
    close(stdout_pipe[pipe_write]);
    throw std::system_error(errno, std::generic_category(),
                            "failed to create child");
  }
  // return child_pid;
  return handle(child_pid, stdin_pipe[pipe_write], stdout_pipe[pipe_read]);
}

} // namespace process
