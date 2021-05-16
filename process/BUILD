cc_library(
    name = "command",
    srcs = [
        "command.cpp",
        "fdbuf.cpp",
    ],
    hdrs = [
        "command.hpp",
        "fdbuf.hpp",
        "fdstream.hpp",
    ],
)

cc_test(
    name = "command_test",
    srcs = [
        "command_test.cpp",
    ],
    deps = [
        ":command",
        "@com_google_googletest//:gtest_main",
    ],
)
