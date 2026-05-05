load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "mlogpp",
    srcs = [
        ":registry_impl.cpp",
    ],
    hdrs = [
        ":fmt_string_with_location.hpp",
        ":formatter.hpp",
        ":level.hpp",
        ":logger.hpp",
        ":record.hpp",
        ":registry.hpp",
        ":sink.hpp",
    ],
    include_prefix = "",
    strip_include_prefix = "",
    visibility = ["//visibility:public"],
)
