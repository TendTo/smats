/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Custom exception classes.
 *
 * This file contains the definition of the custom exception classes.
 * They inherit from std::exception and override the what() method,
 * returning a custom message to aid in debugging.
 */
#pragma once

#include <stdexcept>
#include <string>

namespace smats {

class SmatsException : public std::runtime_error {
 public:
  SmatsException() = default;
  explicit SmatsException(const char* const message) : std::runtime_error(message) {}
  explicit SmatsException(const std::string& message) : std::runtime_error(message) {}
};

class SmatsNotImplementedException : public SmatsException {
 public:
  const char* what() const noexcept override { return "Not implemented"; }
};

class SmatsNotSupported : public SmatsException {
 public:
  const char* what() const noexcept override { return "Not supported"; }
};

class SmatsInvalidArgument : public SmatsException {
 public:
  const char* what() const noexcept override { return "Invalid argument"; }
};

class SmatsInvalidCommandLineArgument : public SmatsException {};

class SmatsInvalidState : public SmatsException {
 public:
  const char* what() const noexcept override { return "Invalid state"; }
};

class SmatsAssertionError : public SmatsException {
 public:
  explicit SmatsAssertionError(const char* const message) : SmatsException(message) {}
  explicit SmatsAssertionError(const std::string& message) : SmatsException(message) {}
};

class SmatsOutOfRange : public SmatsException {
 public:
  explicit SmatsOutOfRange(const char* const message) : SmatsException(message) {}
  explicit SmatsOutOfRange(const std::string& message) : SmatsException(message) {}
};

class SmatsUnreachable : public SmatsException {
 public:
  explicit SmatsUnreachable(const char* const message) : SmatsException(message) {}
  explicit SmatsUnreachable(const std::string& message) : SmatsException(message) {}
};

}  // namespace smats
