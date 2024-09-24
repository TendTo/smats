/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <fmt/core.h>
#include <fmt/format.h>

#include "smats/symbolic/symbolic.h"
#include "smats/util/fmt_ostream.h"
#include "smats/version.h"

using Expression = smats::Expression<double>;
using Variable = smats::Variable;

int main() {
  fmt::println("Hello world!");
  fmt::println("Version: {}", SMATS_VERSION_STRING);

  fmt::println("Arguments: {}", smats::ExpressionKind::Constant);

  const Expression e{3.0};
  const Expression p = Expression{(Expression{Variable{"x"}} + Variable{"y"}) ^ -1};
  const Expression ep = p.expand();
  fmt::println("Arguments: {}", p);
  fmt::println("Arguments: {}", ep);
  fmt::println("e == ep: {}", p.equal_to(ep));
  fmt::println("Arguments: {}", Expression{(Expression{Variable{"x"}} + Variable{"y"}) ^ -1}.expand());
  fmt::println("Arguments: {}", Expression{3.0}.expand());
  fmt::println("Arguments: {}", e.is_expanded());
  fmt::println("Arguments: {}", e.equal_to(e.expand()));
}
