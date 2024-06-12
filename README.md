# Smats

[![smats CI](https://github.com/TendTo/bazel-cpp-template/actions/workflows/smats.yml/badge.svg)](https://github.com/TendTo/bazel-cpp-template/actions/workflows/smats.yml)
[![Docs CI](https://github.com/TendTo/bazel-cpp-template/actions/workflows/docs.yml/badge.svg)](https://github.com/TendTo/bazel-cpp-template/actions/workflows/docs.yml)

Satisfability Modulo Arithmetic Theories Symbols (Smats) provides a simple to use symbolic representation of arithmetic expressions for SMT (Satisfiability Modulo Theories) solvers.

## Utility commands

```bash
# Build the main application.
# The executable can be found in the bazel-bin/smats directory
bazel build //smats
```

```bash
# Run the main application and pass an argument (e.g. 2)
bazel run //smats -- 2
```

```bash
# Build the main application in debug mode
bazel build //smats --config=dbg
```

```bash
# Build the main application in release mode
bazel run //smats --config=opt
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
bazel test //smats/...
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
bazel query --noimplicit_deps --notool_deps 'deps(//smats)'
```

