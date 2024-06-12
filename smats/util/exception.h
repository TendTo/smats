/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Utilities that verify assumptions made by the program and aborts
 * the program if those assumptions are not true.
 *
 * If NDEBUG is defined, most of the macro do nothing and give no explanation.
 * It makes the program faster, but it becomes less useful for debugging.
 */
#pragma once

#include <stdexcept>

#ifdef NDEBUG

#define SMATS_ASSERT(condition, msg) ((void)0)
#define SMATS_ASSERT_FMT(condition, msg, ...) ((void)0)
#define SMATS_UNREACHABLE() std::terminate()
#define SMATS_RUNTIME_ERROR(msg) throw std::runtime_error(msg)
#define SMATS_RUNTIME_ERROR_FMT(msg, ...) throw std::runtime_error(msg)
#define SMATS_OUT_OF_RANGE_FMT(msg, ...) throw std::out_of_range(msg)
#define SMATS_INVALID_ARGUMENT(argument, actual) throw std::runtime_error(argument)
#define SMATS_INVALID_ARGUMENT_EXPECTED(argument, actual, expected) throw std::runtime_error(argument)

#else

#include <fmt/core.h>

#include "smats/util/logging.h"

#define SMATS_ASSERT(condition, message)                                                                 \
  do {                                                                                                     \
    if (!(condition)) {                                                                                    \
      SMATS_CRITICAL_FMT("Assertion `{}` failed in {}:{}: {}", #condition, __FILE__, __LINE__, message); \
      std::terminate();                                                                                    \
    }                                                                                                      \
  } while (false)

#define SMATS_ASSERT_FMT(condition, message, ...)                                           \
  do {                                                                                        \
    if (!(condition)) {                                                                       \
      SMATS_CRITICAL_FMT("Assertion `{}` failed in {}:{}", #condition, __FILE__, __LINE__); \
      SMATS_CRITICAL_FMT(message, __VA_ARGS__);                                             \
      std::terminate();                                                                       \
    }                                                                                         \
  } while (false)

#define SMATS_UNREACHABLE()                                                   \
  do {                                                                          \
    SMATS_CRITICAL_FMT("{}:{} Should not be reachable.", __FILE__, __LINE__); \
    std::terminate();                                                           \
  } while (false)

#define SMATS_RUNTIME_ERROR(msg) \
  do {                             \
    SMATS_CRITICAL(msg);         \
    throw std::runtime_error(msg); \
  } while (false)

#define SMATS_RUNTIME_ERROR_FMT(msg, ...)                  \
  do {                                                       \
    SMATS_CRITICAL_FMT(msg, __VA_ARGS__);                  \
    throw std::runtime_error(fmt::format(msg, __VA_ARGS__)); \
  } while (false)

#define SMATS_OUT_OF_RANGE_FMT(msg, ...)                  \
  do {                                                      \
    SMATS_CRITICAL_FMT(msg, __VA_ARGS__);                 \
    throw std::out_of_range(fmt::format(msg, __VA_ARGS__)); \
  } while (false)

#define SMATS_INVALID_ARGUMENT(argument, actual) \
  throw std::invalid_argument(fmt::format("Invalid argument for {}: {}", argument, actual))

#define SMATS_INVALID_ARGUMENT_EXPECTED(argument, actual, expected) \
  throw std::invalid_argument(                                        \
      fmt::format("Invalid argument for {}: received '{}', expected '{}'", argument, actual, expected))

#endif  // NDEBUG
