# Bazel Cpp Template

[![myapp CI](https://github.com/TendTo/bazel-cpp-template/actions/workflows/myapp.yml/badge.svg)](https://github.com/TendTo/bazel-cpp-template/actions/workflows/myapp.yml)
[![Docs CI](https://github.com/TendTo/bazel-cpp-template/actions/workflows/docs.yml/badge.svg)](https://github.com/TendTo/bazel-cpp-template/actions/workflows/docs.yml)

Simple modern template for a C++ project using Bazel with modules.

## Folder structure

The folder structure is as follows:

```bash
.
├── docs                    # Documentation
│   └── BUILD.bazel         # Documentation BUILD file
├── myapp                   # Main application
│   ├── util                # Utility library
│   │   ├── BUILD.bazel     # Utility library BUILD file
│   │   ├── calculator.cpp
│   │   └── calculator.h
│   ├── BUILD.bazel         # Main application BUILD file
│   └── main.cpp
├── third_party             # Third party libraries
│   ├── BUILD.bazel         # Third party libraries BUILD file
│   └── spdlog.BUILD.bazel  # spdlog BUILD file
├── tools                   # Tools and scripts
│   ├── BUILD.bazel
│   ├── cpplint.bzl         # cpplint rules
│   ├── generate_version_header.bat # Script to generate version header (Windows)
│   ├── generate_version_header.sh  # Script to generate version header (Linux)
│   ├── git_archive.bzl
│   ├── rules_cc.bzl
│   ├── workspace_status.bat # Script to generate the stable-status.txt file (Windows)
│   └── workspace_status.sh  # Script to generate the stable-status.txt file (Linux)
├── .bazelignore    # Bazel ignore file
├── .bazelrc        # Bazel configuration file
├── .bazelversion   # Bazel version lock file
├── .clang-format   # Clang format configuration file
├── CPPLINT.cfg     # cpplint configuration file
├── BUILD.bazel     # Root BUILD file
└── MODULE.bazel    # Root MODULE file
```

## Utility commands

```bash
# Build the main application.
# The executable can be found in the bazel-bin/myapp directory
bazel build //myapp
```

```bash
# Run the main application and pass an argument (e.g. 2)
bazel run //myapp -- 2
```

```bash
# Build the main application in debug mode
bazel build //myapp --config=dbg
```

```bash
# Build the main application in release mode
bazel run //myapp --config=opt
```

```bash
# Run all the tests
bazel test //tests/...
```

```bash
# Only run a specific tagged test
bazel test //tests/... --test_tag_filters=calculator
```

```bash
# Lint all the code
bazel test //myapp/...
```

```bash
# Build the documentation
# The documentation can be found in the bazel-bin/docs directory
bazel build //docs
```

```bash
# Remove all the build files
bazel clean
```

```bash
# Get some information about the cpp toolchain as bazel sees it
bazel cquery "@bazel_tools//tools/cpp:compiler" --output starlark --starlark:expr 'providers(target)'
```

```bash
# Get the dependencies of the main application.
# See https://docs.bazel.build/versions/main/query-how-to.html
bazel query --noimplicit_deps --notool_deps 'deps(//myapp)'
```

## Troubleshooting

### Bazel server stuck

Sometimes, especially on windows, bazel can get stuck compiling without a clear reason why.
If this happens, the bazel server may become unavailable and unable to run any other command.
To solve the situation, you can kill the bazel server and restart it.

```bash
# Get the bazel server PID.
# If it is stuck, it will return something like:
# Another command (pid=7760) is running. Waiting for it to complete on the server (server_pid=8032)...
bazel info server_pid
```

```bash
# Kill the bazel server (Windows)
taskkill /F /PID <server_pid>
# Kill the bazel server (Linux)
kill -9 <server_pid>
```

Any other command will restart the bazel server automatically.
