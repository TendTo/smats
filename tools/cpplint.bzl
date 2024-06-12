"""Used to lint all C++ files in the targets of a BUILD file"""

load("@rules_python//python:defs.bzl", "py_test")

_SOURCE_EXTENSIONS = [
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".c++",
    ".C",
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
    ".inc",
]

# Do not lint generated protocol buffer files.
_IGNORE_EXTENSIONS = [
    ".pb.h",
    ".pb.cc",
]

# The cpplint.py command-line argument so it doesn't skip our files!
_EXTENSIONS_ARGS = ["--extensions=" + ",".join(
    [ext[1:] for ext in _SOURCE_EXTENSIONS],
)]

def _extract_labels(srcs):
    """Convert a srcs= or hdrs= value to its set of labels."""

    # Tuples are already labels.
    if type(srcs) == type(()):
        return list(srcs)

    # The select() syntax returns an object we (apparently) can't inspect.
    # Figure out how to cpplint these files.
    # For now, folks will have to pass extra_srcs when calling cpplint() macro.
    return []

def _is_source_label(label):
    for extension in _IGNORE_EXTENSIONS:
        if label.endswith(extension):
            return False
    for extension in _SOURCE_EXTENSIONS:
        if label.endswith(extension):
            return True
    return False

def _add_linter_rules(source_labels, source_filenames, name, data = None):
    # Common attributes for all of our py_test invocations.
    data = (data or [])

    # Google cpplint.
    py_test(
        name = name + "_cpplint",
        srcs = ["@cpplint"],
        data = data + source_labels + ["//:CPPLINT.cfg"],
        args = _EXTENSIONS_ARGS + source_filenames,
        main = "@cpplint//:cpplint.py",
        size = "small",
        tags = ["cpplint"],
    )

def cpplint(data = None, extra_srcs = None):
    """Add a cpplint target for every c++ source file in each target in the BUILD file so far.

    For every rule in the BUILD file so far, adds a test rule that runs
    cpplint over the C++ sources listed in that rule.  Thus, BUILD file authors
    should call this function at the *end* of every C++-related BUILD file.

    By default, only the CPPLINT.cfg from the project root and the current
    directory are used.  Additional configs can be passed in as data labels.

    Sources that are not discoverable through the "sources so far" heuristic can
    be passed in as extra_srcs=[].

    Args:
        data: additional data to include in the py_test() rule.
        extra_srcs: additional sources to lint.
    """

    # Iterate over all rules.
    for rule in native.existing_rules().values():
        # Extract the list of C++ source code labels and convert to filenames.
        candidate_labels = (
            _extract_labels(rule.get("srcs", ())) +
            _extract_labels(rule.get("hdrs", ()))
        )
        source_labels = [label for label in candidate_labels if _is_source_label(label)]
        source_filenames = ["$(location %s)" % x for x in source_labels]

        if len(source_filenames) == 0:
            continue

        # Run the cpplint checker as a single unit test.
        _add_linter_rules(source_labels, source_filenames, rule["name"], data)

    # Lint all of the extra_srcs separately in a single rule.
    if extra_srcs:
        source_labels = extra_srcs
        source_filenames = ["$(location %s)" % x for x in source_labels]
        _add_linter_rules(
            source_labels,
            source_filenames,
            "extra_srcs_cpplint",
            data,
        )
