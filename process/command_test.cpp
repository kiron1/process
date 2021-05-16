
#include "process/command.hpp"
#include <gtest/gtest.h>

TEST(command, program_path) {
  {
    const auto cmd = process::command("sh");
    EXPECT_EQ(cmd.program_path(), "/bin/sh");
  }
  const auto cmd = process::command("___command_which_does_not_exist___");
  EXPECT_EQ(cmd.program_path(), "");
}

TEST(command, subprocess_exit_code) {
  {
    auto cmd = process::command("/bin/sh").arg("-c").arg("exit");
    auto h = cmd.run();
    auto rc = h.join();
    EXPECT_EQ(rc, 0);
  }

  {
    auto cmd = process::command("/bin/sh").arg("-c").arg("exit 1");
    auto h = cmd.run();
    auto rc = h.join();
    EXPECT_EQ(rc, 1);
  }
}

TEST(command, subprocess_echo) {
  auto cmd = process::command("/bin/sh").arg("-c").arg("echo Hello World!");
  auto h = cmd.run();
  auto line = std::string{};
  EXPECT_TRUE(std::getline(h.stdout(), line));
  EXPECT_EQ(line, "Hello World!");
  auto rc = h.join();
  EXPECT_EQ(rc, 0);
}

TEST(command, subprocess_write_and_read) {
  auto cmd = process::command("/bin/sh").arg("-c").arg("cat -");
  auto h = cmd.run();
  h.stdin() << "Hello World!";
  h.stdin().close();
  auto line = std::string{};
  EXPECT_TRUE(std::getline(h.stdout(), line));
  EXPECT_EQ(line, "Hello World!");
  auto rc = h.join();
  EXPECT_EQ(rc, 0);
}
