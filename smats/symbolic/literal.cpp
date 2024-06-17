/**
 * @author smats
 * @date 15 Aug 2023
 * @copyright 2023 smats
 */

#include "literal.h"

#include <algorithm>

namespace smats {

bool operator==(const smats::Literal &lhs, const smats::Literal &rhs) {
  return lhs.var.equal_to(rhs.var) && lhs.truth == rhs.truth;
}
std::strong_ordering operator<=>(const smats::Literal &lhs, const smats::Literal &rhs) {
  if (lhs.var.get_id() < rhs.var.get_id()) return std::strong_ordering::less;
  if (lhs.var.get_id() > rhs.var.get_id()) return std::strong_ordering::greater;
  if (lhs.truth < rhs.truth) return std::strong_ordering::less;
  if (lhs.truth > rhs.truth) return std::strong_ordering::greater;
  return std::strong_ordering::equal;
}

bool operator==(const smats::LiteralSet &lhs, const smats::LiteralSet &rhs) {
  return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
std::strong_ordering operator<=>(const smats::LiteralSet &lhs, const smats::LiteralSet &rhs) {
  return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

std::ostream &operator<<(std::ostream &os, const Literal &literal) {
  return os << (literal.truth ? "" : "¬") << literal.var;
}

std::ostream &operator<<(std::ostream &os, const LiteralSet &literal_set) {
  os << "{ ";
  for (const auto &lit : literal_set) os << lit << " ";
  return os << "}";
}

std::ostream &operator<<(std::ostream &os, const std::vector<Variable> &variables) {
  os << "[ ";
  for (const auto &var : variables) os << var << " ";
  return os << "]";
}

std::ostream &operator<<(std::ostream &os, const std::vector<Literal> &literals) {
  os << "[ ";
  for (const auto &lit : literals) os << lit << " ";
  return os << "]";
}

std::ostream &operator<<(std::ostream &os, const Model &model) {
  os << "Boolean model:\n";
  for (const auto &lit : model.first) os << lit << " ";
  os << "\nTheory model:\n";
  for (const auto &lit : model.second) os << lit << " ";
  return os;
}
}  // namespace smats
