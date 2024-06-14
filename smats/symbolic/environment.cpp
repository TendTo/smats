/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/environment.h"

#include <utility>

#include "smats/util/exception.h"

namespace smats {

namespace {

/**
 * Given a list of variables, @p vars, builds an Environment::map which maps a
 * Variable to its double value. All values are set to 0.0.
 * @param vars list of variables
 * @return Environment::map
 */
Environment::map BuildMap(const std::initializer_list<Environment::key_type> vars) {
  Environment::map m;
  for (const Environment::key_type& var : vars) m.emplace(var, 0.0);
  return m;
}

}  // namespace

Environment::Environment(const std::initializer_list<value_type> init) : Environment{map(init)} {}

Environment::Environment(const std::initializer_list<key_type> vars) : Environment{BuildMap(vars)} {}

Environment::Environment(map m) : map_{std::move(m)} {
  if (std::any_of(map_.begin(), map_.end(), [](const auto& p) { return p.first.is_dummy(); }))
    SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
}

void Environment::insert(const key_type& key, const mapped_type& elem) {
  if (key.is_dummy()) SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
  map_.emplace(key, elem);
}

void Environment::insert_or_assign(const key_type& key, const mapped_type& elem) {
  if (key.is_dummy()) SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
  map_.insert_or_assign(key, elem);
}

Variables Environment::domain() const {
  Variables dom;
  for (const auto& p : map_) dom += p.first;
  return dom;
}

const Environment::mapped_type& Environment::at(const key_type& key) const { return map_.at(key); }

Environment::mapped_type& Environment::operator[](const key_type& key) { return map_[key]; }

const Environment::mapped_type& Environment::operator[](const key_type& key) const { return map_.at(key); }

std::ostream& operator<<(std::ostream& os, const Environment& env) {
  for (const auto& [var, value] : env) os << var << " -> " << value << ", ";
  return os;
}

}  // namespace smats
