/**
 * @author smats
 * @date 15 Aug 2023
 * @copyright 2023 smats
 * @brief Brief description
 *
 * Long Description
 */
#pragma once

#include <compare>
#include <ostream>
#include <set>
#include <utility>
#include <vector>

#include "smats/symbolic/symbolic_variable.h"  // IWYU pragma: export
#include "smats/util/logging.h"

namespace smats {

using drake::symbolic::Variable;

struct Literal {
  Variable var;
  bool truth;
};
using LiteralSet = std::set<Literal>;                                 //< A set of literals.
using Model = std::pair<std::vector<Literal>, std::vector<Literal>>;  //< A model is a pair of vectors of literals.

bool operator==(const smats::Literal &lhs, const smats::Literal &rhs);
std::strong_ordering operator<=>(const smats::Literal &lhs, const smats::Literal &rhs);

bool operator==(const smats::LiteralSet &lhs, const smats::LiteralSet &rhs);
std::strong_ordering operator<=>(const smats::LiteralSet &lhs, const smats::LiteralSet &rhs);

std::ostream &operator<<(std::ostream &os, const std::vector<Variable> &variables);
std::ostream &operator<<(std::ostream &os, const Literal &literal);
std::ostream &operator<<(std::ostream &os, const LiteralSet &literals);
std::ostream &operator<<(std::ostream &os, const Model &model);
std::ostream &operator<<(std::ostream &os, const std::vector<Literal> &variables);

}  // namespace smats

OSTREAM_FORMATTER(smats::Variable)
OSTREAM_FORMATTER(smats::Variable::Type)
OSTREAM_FORMATTER(smats::Literal)
OSTREAM_FORMATTER(smats::LiteralSet)
OSTREAM_FORMATTER(smats::Model)
