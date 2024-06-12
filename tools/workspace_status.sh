#!/bin/bash
# Script called upon compilation by Bazel.
# It stores useful information about the compilation environment
# in a file called "bazel-out/stable-status.txt".
# This file is then read by scripts/generate_version_header.sh to
# generate the version header file.
# The invocation of this script is defined in the file .bazelrc.
# See https://bazel.build/docs/user-manual#workspace-status
# for more information.
# Note: This script is called by Bazel. So it should not depend on
# any external tools.
echo "CURRENT_TIME $(date +%s)"
echo "STABLE_GIT_COMMIT $(git rev-parse HEAD)"
echo "STABLE_USER_NAME $USER"
echo "STABLE_HOST_NAME $(hostname)"
echo "STABLE_REPOSITORY_STATUS $(git describe --tags --dirty 2>/dev/null || echo 'unknown')"
echo "STABLE_REPOSITORY_STATUS_SHORT $(git describe --tags --dirty --always 2>/dev/null || echo 'unknown')"
echo "STABLE_REPOSITORY_STATUS_LONG $(git describe --tags --dirty --long 2>/dev/null || echo 'unknown')"
