/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <fmt/core.h>

#include <iostream>

#include "smats/version.h"

int main(int argc, char **argv) {
  fmt::println("Hello world!");
  fmt::println("Version: {}", SMATS_VERSION_STRING);
}
