/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/util/exception.h"

#include <gtest/gtest.h>
#include <stdexcept>

using std::runtime_error;
using std::invalid_argument;

class TestException : public ::testing::Test {
 protected:
  void SetUp() override { SMATS_LOG_INIT_VERBOSITY(5); }
  void TearDown() override { SMATS_LOG_INIT_VERBOSITY(-1); }
};

TEST_F(TestException, AssertFail) {
  EXPECT_DEATH(SMATS_ASSERT(false, "Message"), "Assertion `false` failed");
}

TEST_F(TestException, AssertFailReport) {
  EXPECT_DEATH(SMATS_ASSERT(1 + 1 == 3, "Message"), "Message");
}

TEST_F(TestException, AssertSuccess) {
  EXPECT_NO_THROW(SMATS_ASSERT(true, "Message"));
}

TEST_F(TestException, Unreachable) {
  EXPECT_DEATH(SMATS_UNREACHABLE(), "Should not be reachable");
}

TEST_F(TestException, RuntimeError) {
  EXPECT_THROW(SMATS_RUNTIME_ERROR("Message"), runtime_error);
}

TEST(TestLogging, RuntimeErrorFmt) {
  EXPECT_THROW(SMATS_RUNTIME_ERROR_FMT("Message: {}", "format"), runtime_error);
}
