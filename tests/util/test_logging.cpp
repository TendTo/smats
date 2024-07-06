/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>
#include "smats/util/logging.h"

TEST(TestLogging, Info) {
    EXPECT_NO_THROW(SMATS_INFO("TestLogging::Info"));
}

TEST(TestLogging, InfoFmt) {
    EXPECT_NO_THROW(SMATS_INFO_FMT("TestLogging::Info{}", "Fmt"));
}

TEST(TestLogging, Trace) {
    EXPECT_NO_THROW(SMATS_TRACE("TestLogging::Trace"));
}

TEST(TestLogging, TraceFmt) {
    EXPECT_NO_THROW(SMATS_TRACE_FMT("TestLogging::Trace{}", "Fmt"));
}

TEST(TestLogging, Debug) {
    EXPECT_NO_THROW(SMATS_DEBUG("TestLogging::Debug"));
}

TEST(TestLogging, DebugFmt) {
    EXPECT_NO_THROW(SMATS_DEBUG_FMT("TestLogging::Debug{}", "Fmt"));
}

TEST(TestLogging, Warn) {
    EXPECT_NO_THROW(SMATS_WARN("TestLogging::Warn"));
}

TEST(TestLogging, WarnFmt) {
    EXPECT_NO_THROW(SMATS_WARN_FMT("TestLogging::Warn{}", "Fmt"));
}

TEST(TestLogging, Error) {
    EXPECT_NO_THROW(SMATS_ERROR("TestLogging::Error"));
}

TEST(TestLogging, ErrorFmt) {
    EXPECT_NO_THROW(SMATS_ERROR_FMT("TestLogging::Error{}", "Fmt"));
}

TEST(TestLogging, Critical) {
    EXPECT_NO_THROW(SMATS_CRITICAL("TestLogging::Critical"));
}

TEST(TestLogging, CriticalFmt) {
    EXPECT_NO_THROW(SMATS_CRITICAL_FMT("TestLogging::Critical{}", "Fmt"));
}

TEST(TestLogging, LogInit) {
    EXPECT_NO_THROW(SMATS_LOG_INIT_LEVEL(spdlog::level::trace));
}

TEST(TestLogging, VerbosityToLogLevel) {
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(0), spdlog::level::critical);
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(1), spdlog::level::err);
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(2), spdlog::level::warn);
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(3), spdlog::level::info);
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(4), spdlog::level::debug);
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(5), spdlog::level::trace);
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(6), spdlog::level::off);
    EXPECT_EQ(SMATS_VERBOSITY_TO_LOG_LEVEL(-1), spdlog::level::off);
}