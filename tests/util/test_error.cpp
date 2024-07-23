/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include <stdexcept>

#include "smats/util/error.h"

using smats::SmatsAssertionError;
using smats::SmatsException;
using smats::SmatsUnreachable;

TEST(TestException, AssertFail) {
#ifdef NDEBUG
  EXPECT_NO_THROW(SMATS_ASSERT(false, "Message"));
#else
  EXPECT_THROW(SMATS_ASSERT(false, "Message"), SmatsAssertionError);
#endif
}

TEST(TestException, AssertSuccess) { EXPECT_NO_THROW(SMATS_ASSERT(true, "Message")); }

TEST(TestException, Unreachable) {
#ifdef NDEBUG
  EXPECT_DEATH(SMATS_UNREACHABLE(), "");
#else
  EXPECT_THROW(SMATS_UNREACHABLE(), SmatsUnreachable);
#endif
}

TEST(TestException, RuntimeError) { EXPECT_THROW(SMATS_RUNTIME_ERROR("Message"), SmatsException); }

TEST(TestLogging, RuntimeErrorFmt) { EXPECT_THROW(SMATS_RUNTIME_ERROR_FMT("Message: {}", "format"), SmatsException); }
