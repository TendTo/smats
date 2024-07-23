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

#include <exception>

namespace smats {

class SmatsException : public std::exception {
 public:
  SmatsException() = default;
  SmatsException(const char* const message) : std::exception(message) {}
  SmatsException(const std::string& message) : std::exception(message.c_str()) {}
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
  SmatsAssertionError(const char* const message) : SmatsException(message) {}
  SmatsAssertionError(const std::string& message) : SmatsException(message.c_str()) {}
};

class SmatsOutOfRange : public SmatsException {
 public:
  SmatsOutOfRange(const char* const message) : SmatsException(message) {}
  SmatsOutOfRange(const std::string& message) : SmatsException(message.c_str()) {}
};

class SmatsUnreachable : public SmatsException {
 public:
  SmatsUnreachable(const char* const message) : SmatsException(message) {}
  SmatsUnreachable(const std::string& message) : SmatsException(message.c_str()) {}
};

}  // namespace smats
