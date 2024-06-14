/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/variables.h"

#include <iterator>
#include <numeric>
#include <utility>

#include "smats/util/hash.hpp"

namespace smats {

Variables::Variables(std::initializer_list<Variable> init) : vars_(init) {}

Variables::size_type Variables::erase(const Variables& vars) {
  size_type num_of_erased_elements{0};
  for (const Variable& var : vars) num_of_erased_elements += erase(var);
  return num_of_erased_elements;
}

bool Variables::IsSubsetOf(const Variables& vars) const {
  return std::includes(vars.begin(), vars.end(), begin(), end(), std::less<Variable>{});
}

bool Variables::IsSupersetOf(const Variables& vars) const { return vars.IsSubsetOf(*this); }

bool Variables::IsStrictSubsetOf(const Variables& vars) const {
  if (*this == vars) return false;
  return IsSubsetOf(vars);
}

bool Variables::IsStrictSupersetOf(const Variables& vars) const {
  if (*this == vars) return false;
  return IsSupersetOf(vars);
}

bool Variables::operator==(const Variables& vars) const {
  return std::equal(vars_.begin(), vars_.end(), vars.vars_.begin(), vars.vars_.end(), std::equal_to<Variable>{});
}

bool Variables::operator<(const Variables& vars) const {
  return std::lexicographical_compare(vars_.begin(), vars_.end(), vars.vars_.begin(), vars.vars_.end(),
                                      std::less<Variable>{});
}

Variables& Variables::operator+=(const Variables& vars) {
  vars_.insert(vars.begin(), vars.end());
  return *this;
}
Variables& Variables::operator+=(const Variable& var) {
  vars_.insert(var);
  return *this;
}
Variables Variables::operator+(const Variables& vars) const {
  Variables temp{vars_};
  temp += vars;
  return temp;
}
Variables Variables::operator+(const Variable& var) const {
  Variables temp{vars_};
  temp += var;
  return temp;
}
Variables operator+(const Variable& var, Variables vars) {
  vars += var;
  return vars;
}

// NOLINTNEXTLINE(runtime/references) per C++ standard signature.
Variables& Variables::operator-=(const Variables& vars) {
  for (const Variable& var : vars) vars_.erase(var);
  return *this;
}
// NOLINTNEXTLINE(runtime/references) per C++ standard signature.
Variables& Variables::operator-=(const Variable& var) {
  vars_.erase(var);
  return *this;
}
Variables Variables::operator-(const Variables& vars) const {
  Variables temp{vars_};
  temp -= vars;
  return temp;
}
Variables Variables::operator-(const Variable& var) const {
  Variables temp{vars_};
  temp -= var;
  return temp;
}

Variables::Variables(std::set<Variable> vars) : vars_{std::move(vars)} {}

Variables Variables::intersect(const Variables& vars) const {
  std::set<Variable> intersection;
  std::set_intersection(vars_.begin(), vars_.end(), vars.vars_.begin(), vars.vars_.end(),
                        std::inserter(intersection, intersection.begin()), std::less<Variable>{});
  return Variables{std::move(intersection)};
}

std::ostream& operator<<(std::ostream& os, const Variables& vars) {
  os << "{";
  if (!vars.empty()) {
    // output 1st ... N-1th elements by adding ", " at the end
    copy(vars.begin(), prev(vars.end()), std::ostream_iterator<Variable>(os, ", "));
    // output the last one (without ",").
    os << *(vars.rbegin());
  }
  os << "}";
  return os;
}

}  // namespace smats
