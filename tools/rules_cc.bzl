"""Provide a centralised place to define rules for building C++ code.
This makes it easy to change the build configuration for all C++ rules in the project at once.
Inspired by Drake's drake.bzl file https://github.com/RobotLocomotion/drake/blob/master/tools/drake.bzl.
"""

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

MYAPP_NAME = "myapp"
MYAPP_VERSION = "0.0.1"

# The CXX_FLAGS will be enabled for all C++ rules in the project
# building with any linux compiler.
CXX_FLAGS = [
    "-std=c++20",
    "-Wall",
    "-Wattributes",
    "-Wdeprecated",
    "-Wdeprecated-declarations",
    "-Wextra",
    "-Wignored-qualifiers",
    "-Wold-style-cast",
    "-Woverloaded-virtual",
    "-Wpedantic",
    "-Wshadow",
    "-Werror",
]

# The CLANG_FLAGS will be enabled for all C++ rules in the project when
# building with clang.
CLANG_FLAGS = CXX_FLAGS + [
    "-Wabsolute-value",
    "-Winconsistent-missing-override",
    "-Wnon-virtual-dtor",
    "-Wreturn-stack-address",
    "-Wsign-compare",
]

# The GCC_FLAGS will be enabled for all C++ rules in the project when
# building with gcc.
GCC_FLAGS = CXX_FLAGS + [
    "-Wlogical-op",
    "-Wnon-virtual-dtor",
    "-Wreturn-local-addr",
    "-Wunused-but-set-parameter",
]

# The MSVC_CL_FLAGS will be enabled for all C++ rules in the project when
# building with MSVC.
MSVC_CL_FLAGS = [
    "-W4",
    "-WX",
    # "-wd4068",  # unknown pragma
]

# The CLANG_CL_FLAGS will be enabled for all C++ rules in the project when
# building with clang-cl.
CLANG_CL_FLAGS = [
    "-Wabsolute-value",
    "-Winconsistent-missing-override",
    "-Wnon-virtual-dtor",
    "-Wreturn-stack-address",
    "-Wsign-compare",
]

# The test configuration flags for each compiler.
GCC_TEST_FLAGS = []
CLANG_TEST_FLAGS = []
MSVC_CL_TEST_FLAGS = []
CLANG_CL_TEST_FLAGS = []

# Default defines for all C++ rules in the project.
MYAPP_DEFINES = []

def _get_copts(rule_copts, cc_test = False):
    """Alter the provided rule specific copts, adding the platform-specific ones.

    When cc_test is True, the corresponding test flags will be added.
    It should only be set on cc_test rules or rules that are boil down to cc_test rules.

    Args:
        rule_copts: The copts passed to the rule.
        cc_test: Whether the rule is a cc_test rule.

    Returns:
        A list of copts.
    """
    return select({
        "//tools:gcc_build": GCC_FLAGS + GCC_TEST_FLAGS if cc_test else [] + rule_copts,
        "//tools:clang_build": CLANG_FLAGS + CLANG_CL_TEST_FLAGS if cc_test else [] + rule_copts,
        "//tools:msvc_cl_build": MSVC_CL_FLAGS + MSVC_CL_TEST_FLAGS if cc_test else [] + rule_copts,
        "//tools:clang_cl_build": CLANG_CL_FLAGS + CLANG_CL_TEST_FLAGS if cc_test else [] + rule_copts,
        "//conditions:default": CXX_FLAGS + rule_copts,
    })

def _get_defines(rule_defines):
    """Alter the provided rule specific defines, adding the platform-specific ones.

    Args:
        rule_defines: The defines passed to the rule.

    Returns:
        A list of defines.
    """
    return rule_defines + MYAPP_DEFINES + select({
        "//tools:debug_build": [],
        "//conditions:default": [],
    }) + select({
        "//tools:release_build": ["NDEBUG", "NLOG"],
        "//conditions:default": [],
    })

def _get_static(rule_linkstatic):
    """Alter the provided linkstatic, by considering the platform-specific one.

    The files are linked statically by default.

    Args:
        rule_linkstatic: The linkstatic passed to the rule.

    Returns:
        The linkstatic value to use
    """
    if rule_linkstatic != None:
        return rule_linkstatic
    return select({
        "//tools:static_build": True,
        "//tools:dynamic_build": False,
        "//conditions:default": True,
    })

def _get_features(rule_features):
    """Alter the provided features, adding the platform-specific ones.

    Args:
        rule_features: The features passed to the rule.

    Returns:
        A list of features.
    """
    return rule_features + select({
        "//tools:dynamic_build": [],
        "//tools:static_build": ["fully_static_link"],
        "//conditions:default": [],
    })

def myapp_cc_library(
        name,
        hdrs = None,
        srcs = None,
        deps = None,
        copts = [],
        linkstatic = None,
        defines = [],
        implementation_deps = [],
        **kwargs):
    """Creates a rule to declare a C++ library.

    Args:
        name: The name of the library.
        hdrs: A list of header files to add. Will be inherited by dependents.
        srcs: A list of source files to compile.
        deps: A list of dependencies. Will be inherited by dependents.
        implementation_deps: A list of dependencies that are only needed for this target.
        copts: A list of compiler options.
        linkstatic: Whether to link statically.
        defines: A list of compiler defines used when compiling this target and its dependents.
        **kwargs: Additional arguments to pass to cc_library.
    """
    cc_library(
        name = name,
        hdrs = hdrs,
        srcs = srcs,
        deps = deps,
        implementation_deps = implementation_deps,
        copts = _get_copts(copts),
        linkstatic = _get_static(linkstatic),
        defines = _get_defines(defines),
        **kwargs
    )

def myapp_cc_binary(
        name,
        srcs = None,
        deps = None,
        copts = [],
        linkstatic = None,
        defines = [],
        features = [],
        **kwargs):
    """Creates a rule to declare a C++ binary.

    Args:
        name: The name of the binary.
        srcs: A list of source files to compile.
        deps: A list of dependencies.
        copts: A list of compiler options.
        linkstatic: Whether to link statically.
        defines: A list of compiler defines used when compiling this target.
        features: A list of features to add to the binary.
        **kwargs: Additional arguments to pass to cc_binary.
    """
    cc_binary(
        name = name,
        srcs = srcs,
        deps = deps,
        copts = _get_copts(copts),
        linkstatic = _get_static(linkstatic),
        defines = _get_defines(defines),
        features = _get_features(features),
        **kwargs
    )

def myapp_cc_test(
        name,
        srcs = None,
        copts = [],
        tags = [],
        defines = [],
        **kwargs):
    """Creates a rule to declare a C++ unit test.

    Note that for almost all cases, myapp_cc_googletest should be used instead of this rule.

    By default, sets size="small" because that indicates a unit test.
    If a list of srcs is not provided, it will be inferred from the name, by capitalizing each _-separated word and appending .cpp.
    For example, myapp_cc_test(name = "test_foo_bar") will look for TestFooBar.cpp.
    Furthermore, a tag will be added for the test, based on the name, by converting the name to lowercase and removing the "test_" prefix.

    Args:
        name: The name of the test.
        srcs: A list of source files to compile.
        copts: A list of compiler options.
        tags: A list of tags to add to the test. Allows for test filtering.
        defines: A list of compiler defines used when compiling this target.
        **kwargs: Additional arguments to pass to cc_test.
    """
    if srcs == None:
        srcs = ["".join([word.capitalize() for word in name.split("_")]) + ".cpp"]
    cc_test(
        name = name,
        srcs = srcs,
        copts = _get_copts(copts, cc_test = True),
        linkstatic = True,
        tags = tags + ["myapp", "".join([word.lower() for word in name.split("_")][1:])],
        defines = _get_defines(defines),
        **kwargs
    )

def myapp_cc_googletest(
        name,
        srcs = None,
        deps = None,
        size = "small",
        tags = [],
        use_default_main = True,
        defines = [],
        **kwargs):
    """Creates a rule to declare a C++ unit test using googletest.

    Always adds a deps= entry for googletest main
    (@googletest//:gtest_main).

    By default, it uses size="small" because that indicates a unit test.
    By default, it uses use_default_main=True to use GTest's main, via @googletest//:gtest_main.
    If use_default_main is False, it will depend on @googletest//:gtest instead.
    If a list of srcs is not provided, it will be inferred from the name, by capitalizing each _-separated word and appending .cpp.
    For example, myapp_cc_test(name = "test_foo_bar") will look for TestFooBar.cpp.
    Furthermore, a tag will be added for the test, based on the name, by converting the name to lowercase and removing the "test_" prefix.

    Args:
        name: The name of the test.
        srcs: A list of source files to compile.
        deps: A list of dependencies.
        size: The size of the test.
        tags: A list of tags to add to the test. Allows for test filtering.
        use_default_main: Whether to use googletest's main.
        defines: A list of compiler defines used when compiling this target.
        **kwargs: Additional arguments to pass to myapp_cc_test.
    """
    if deps == None:
        deps = []
    if type(deps) == "select":
        if use_default_main:
            deps += select({"//conditions:default": ["@googletest//:gtest_main"]})
        else:
            deps += select({"//conditions:default": ["@googletest//:gtest"]})
    elif use_default_main:
        deps.append("@googletest//:gtest_main")
    else:
        deps.append("@googletest//:gtest")
    myapp_cc_test(
        name = name,
        srcs = srcs,
        deps = deps,
        size = size,
        tags = tags + ["googletest"],
        defines = _get_defines(defines),
        **kwargs
    )

def myapp_srcs(name, srcs = None, hdrs = None, deps = [], visibility = ["//visibility:public"]):
    """Returns three different lists of source files based on the name.

    Args:
        name: The name of the target. If the name is "srcs", the default "srcs", "hdrs", and "all_srcs" will be used.
            Otherwise, "srcs_" + name, "hdrs_" + name, and "all_srcs_" + name will be used.
        srcs: A list of source files include in the filegroup. If None, common c++ source files extensions will be used.
        hdrs: A list of header files to include in the filegroup. If None, common c++ header files extensions will be used.
        deps: A list of dependencies. Used for the all_srcs filegroup.
        visibility: A list of visibility labels to apply to the filegroups.
    """
    if name == "srcs":
        srcs_name, hdrs_name, all_srcs_name = "srcs", "hdrs", "all_srcs"
    else:
        srcs_name, hdrs_name, all_srcs_name = "srcs_%s" % name, "hdrs_%s" % name, "all_srcs_%s" % name
    if srcs == None:
        srcs = native.glob(["*.cpp", "*.cc", "*.cxx", "*.c"])
    if hdrs == None:
        hdrs = native.glob(["*.h", "*.hpp"])
    native.filegroup(
        name = srcs_name,
        srcs = srcs + hdrs,
        visibility = visibility,
    )
    native.filegroup(
        name = hdrs_name,
        srcs = hdrs,
        visibility = visibility,
    )
    native.filegroup(
        name = all_srcs_name,
        srcs = srcs + hdrs + deps,
        visibility = visibility,
    )
