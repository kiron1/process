# `process`

A small C++ library to spawn processes on POSIX systems and communicate
with them via [streams](https://en.cppreference.com/w/cpp/io).

## Build

Use [Bazel](https://bazel.build) to build this library.

To build and run tests, use the following command:

```sh
bazel test '//...'
```

## Example

```cpp
auto cmd = process::command("sh").arg("-c").arg("echo Hello World!");
auto h = proc.run();
auto line = std::string{};
std::getline(h.stdout(), line); // => line = "Hello World!"
auto rc = h.join(); // => rc = 0, exit code
```

For more examples see [command_test.cpp](process/command_test.cpp).