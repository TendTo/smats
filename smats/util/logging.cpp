/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */

#include "smats/util/logging.h"

#ifndef NLOG

#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace smats {

std::shared_ptr<spdlog::logger> get_logger(const bool to_stdout) {
  // Checks if there exists a logger with the name. If it exists, return it.
  const char *logger_name = to_stdout ? "smats_out" : "smats_err";
  std::shared_ptr<spdlog::logger> logger{spdlog::get(logger_name)};
  if (logger) return logger;

  // Create and return a new logger.
  logger = to_stdout ? spdlog::stdout_color_mt(logger_name) : spdlog::stderr_color_mt(logger_name);

  // Turn it off by default so that external programs using this as a library do not see internal loggings.
  logger->set_level(spdlog::level::off);

  // Set format.
  logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");

  return logger;
}

}  // namespace smats

#else

#endif  // NLOG
