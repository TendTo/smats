/**
 * @file main.cpp
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024
 * @licence Apache-2.0 license
 */
#include <fmt/core.h>

#include <iostream>

#include "myapp/util/calculator.h"
#include "myapp/version.h"

int main(int argc, char **argv) {
  int verbose = 0;
  if (argc > 1) {
    verbose = std::stoi(argv[1]);
  }

  fmt::println("Hello world!");
  fmt::println("Version: {}", MYAPP_VERSION_STRING);

  fmt::println("Insert the first number: ");
  int a;
  std::cin >> a;
  fmt::println("Insert the second number: ");
  int b;
  std::cin >> b;

  myapp::Calculator calc{verbose};
  fmt::println("Sum: {}", calc.add(a, b));
  fmt::println("Subtraction: {}", calc.subtract(a, b));
  fmt::println("Multiplication: {}", calc.multiply(a, b));
  fmt::println("Division: {}", calc.divide(a, b));
}
