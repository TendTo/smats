/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Ostream formatter for smats
 *
 * Including this file will allow you to use the smats types with fmt::print and fmt::format.
 * If the feature is not enabled, avoiding to include this file will remove the dependency on fmt.
 */
#pragma once

#include <fmt/ostream.h>

#include "smats/symbolic/symbolic.h"

#define OSTREAM_FORMATTER(type) \
  template <>                   \
  struct fmt::formatter<type> : ostream_formatter {};

OSTREAM_FORMATTER(smats::ExpressionKind);
OSTREAM_FORMATTER(smats::Variable);
OSTREAM_FORMATTER(smats::Variables);
OSTREAM_FORMATTER(smats::EnvironmentD);
OSTREAM_FORMATTER(smats::EnvironmentF);
OSTREAM_FORMATTER(smats::EnvironmentI);
OSTREAM_FORMATTER(smats::EnvironmentL);
OSTREAM_FORMATTER(smats::ExpressionD);
OSTREAM_FORMATTER(smats::ExpressionF);
OSTREAM_FORMATTER(smats::ExpressionI);
OSTREAM_FORMATTER(smats::ExpressionL);
