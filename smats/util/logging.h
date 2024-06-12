/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Logging macros.
 * Allows logging with different verbosity levels using spdlog.
 *
 * The verbosity level is set with the --verbosity flag.
 * The verbosity level is an integer between 0 and 5.
 * The higher the verbosity level, the more information is logged.
 * If another value is provided, the logging is turned off.
 */
#pragma once

#ifndef NLOG

#include <fmt/core.h>     // IWYU pragma: export
#include <fmt/ostream.h>  // IWYU pragma: export
#include <fmt/ranges.h>   // IWYU pragma: export
#include <spdlog/logger.h>

#include <memory>

namespace smats {

enum class LoggerType { OUT, ERR };

std::shared_ptr<spdlog::logger> get_logger(LoggerType logger_type);  // NOLINT

}  // namespace smats

#define OSTREAM_FORMATTER(type) \
  template <>                   \
  struct fmt::formatter<type> : ostream_formatter {};
#define SMATS_FORMAT(message, ...) fmt::format(message, __VA_ARGS__)

#define SMATS_VERBOSITY_TO_LOG_LEVEL(verbosity)                      \
  ((verbosity) == 0                                                    \
       ? spdlog::level::critical                                       \
       : ((verbosity) == 1                                             \
              ? spdlog::level::err                                     \
              : ((verbosity) == 2                                      \
                     ? spdlog::level::warn                             \
                     : ((verbosity) == 3                               \
                            ? spdlog::level::info                      \
                            : ((verbosity) == 4 ? spdlog::level::debug \
                                                : ((verbosity) == 5 ? spdlog::level::trace : spdlog::level::off))))))
#define SMATS_LOG_INIT_VERBOSITY(verbosity) SMATS_LOG_INIT_LEVEL(SMATS_VERBOSITY_TO_LOG_LEVEL(verbosity))
#define SMATS_LOG_INIT_LEVEL(level)                                    \
  do {                                                                   \
    ::smats::get_logger(::smats::LoggerType::OUT)->set_level(level); \
    ::smats::get_logger(::smats::LoggerType::ERR)->set_level(level); \
  } while (0)
#define SMATS_TRACE(msg) ::smats::get_logger(::smats::LoggerType::OUT)->trace(msg)
#define SMATS_TRACE_FMT(msg, ...) ::smats::get_logger(::smats::LoggerType::OUT)->trace(msg, __VA_ARGS__)
#define SMATS_DEBUG(msg) ::smats::get_logger(::smats::LoggerType::OUT)->debug(msg)
#define SMATS_DEBUG_FMT(msg, ...) ::smats::get_logger(::smats::LoggerType::OUT)->debug(msg, __VA_ARGS__)
#define SMATS_INFO(msg) ::smats::get_logger(::smats::LoggerType::OUT)->info(msg)
#define SMATS_INFO_FMT(msg, ...) ::smats::get_logger(::smats::LoggerType::OUT)->info(msg, __VA_ARGS__)
#define SMATS_WARN(msg) ::smats::get_logger(::smats::LoggerType::ERR)->warn(msg)
#define SMATS_WARN_FMT(msg, ...) ::smats::get_logger(::smats::LoggerType::ERR)->warn(msg, __VA_ARGS__)
#define SMATS_ERROR(msg) ::smats::get_logger(::smats::LoggerType::ERR)->error(msg)
#define SMATS_ERROR_FMT(msg, ...) ::smats::get_logger(::smats::LoggerType::ERR)->error(msg, __VA_ARGS__)
#define SMATS_CRITICAL(msg) ::smats::get_logger(::smats::LoggerType::ERR)->critical(msg)
#define SMATS_CRITICAL_FMT(msg, ...) ::smats::get_logger(::smats::LoggerType::ERR)->critical(msg, __VA_ARGS__)
#define SMATS_INFO_ENABLED (::smats::get_logger(::smats::LoggerType::OUT)->should_log(spdlog::level::info))
#define SMATS_TRACE_ENABLED (::smats::get_logger(::smats::LoggerType::OUT)->should_log(spdlog::level::trace))

#else

#include <fmt/core.h>  // IWYU pragma: export

#define OSTREAM_FORMATTER(type)
#define SMATS_FORMAT(message, ...) fmt::format(message, __VA_ARGS__)
#define SMATS_VERBOSITY_TO_LOG_LEVEL(verbosity) 0
#define SMATS_LOG_INIT_LEVEL(level) void(0)
#define SMATS_LOG_INIT_VERBOSITY(verbosity) void(0)
#define SMATS_TRACE(msg) void(0)
#define SMATS_TRACE_FMT(msg, ...) void(0)
#define SMATS_DEBUG(msg) void(0)
#define SMATS_DEBUG_FMT(msg, ...) void(0)
#define SMATS_INFO(msg) void(0)
#define SMATS_INFO_FMT(msg, ...) void(0)
#define SMATS_WARN(msg) void(0)
#define SMATS_WARN_FMT(msg, ...) void(0)
#define SMATS_ERROR(msg) void(0)
#define SMATS_ERROR_FMT(msg, ...) void(0)
#define SMATS_CRITICAL(msg) void(0)
#define SMATS_CRITICAL_FMT(msg, ...) void(0)
#define SMATS_INFO_ENABLED false
#define SMATS_TRACE_ENABLED false

#endif
