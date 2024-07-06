/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include <stdexcept>

#include "smats/util/exception.h"

using std::invalid_argument;
using std::runtime_error;

class TestException : public ::testing::Test {
 protected:
#ifndef NDEBUG
  TestException() { SMATS_LOG_INIT_VERBOSITY(5); }
  ~TestException() { SMATS_LOG_INIT_VERBOSITY(-1); }
#endif
};

TEST_F(TestException, AssertFail) {
#ifdef NDEBUG
  EXPECT_NO_THROW(SMATS_ASSERT(false, "Message"));
#elif defined(NLOG)
  EXPECT_DEATH(SMATS_ASSERT(false, "Message"), "Assertion `false` failed");
#else
  EXPECT_DEATH(SMATS_ASSERT(false, "Message"), "");
#endif
}

TEST_F(TestException, AssertSuccess) { EXPECT_NO_THROW(SMATS_ASSERT(true, "Message")); }

TEST_F(TestException, Unreachable) {
#ifndef NLOG
  EXPECT_DEATH(SMATS_UNREACHABLE(), "Should not be reachable");
#else
  EXPECT_DEATH(SMATS_UNREACHABLE(), "");
#endif
}

TEST_F(TestException, RuntimeError) { EXPECT_THROW(SMATS_RUNTIME_ERROR("Message"), runtime_error); }

TEST(TestLogging, RuntimeErrorFmt) { EXPECT_THROW(SMATS_RUNTIME_ERROR_FMT("Message: {}", "format"), runtime_error); }
