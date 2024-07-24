/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <fmt/core.h>
#include <fmt/format.h>

#include <iostream>

#include "smats/symbolic/symbolic.h"
#include "smats/util/fmt_ostream.h"
#include "smats/version.h"

using Expression = smats::Expression<double>;

int main(int argc, char **argv) {
  fmt::println("Hello world!");
  fmt::println("Version: {}", SMATS_VERSION_STRING);

  fmt::println("Arguments: {}", smats::ExpressionKind::Constant);

  fmt::println("Arguments: {}", Expression{3.0});
}
