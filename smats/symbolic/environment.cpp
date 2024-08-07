/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/environment.h"

#include <utility>

#include "smats/util/error.h"

namespace smats {

namespace {

/**
 * Given a list of variables, @p vars, builds an Environment::map which maps a
 * Variable to its double value. All values are set to 0.0.
 * @param vars list of variables
 * @return Environment::map
 */
template <class T>
typename Environment<T>::map BuildMap(const std::span<const typename Environment<T>::key_type> vars) {
  typename Environment<T>::map m;
  for (const typename Environment<T>::key_type& var : vars) m.emplace(var, 0.0);
  return m;
}

}  // namespace

template <class T>
Environment<T>::Environment(const std::initializer_list<value_type> init) : Environment<T>{map(init)} {}

template <class T>
Environment<T>::Environment(std::span<const value_type> init) : Environment{map(init.begin(), init.end())} {}

template <class T>
Environment<T>::Environment(const std::initializer_list<key_type> vars)
    : Environment{BuildMap<T>(std::span<const key_type>(vars.begin(), vars.end()))} {}

template <class T>
Environment<T>::Environment(std::span<const key_type> vars) : Environment{BuildMap<T>(vars)} {}

template <class T>
Environment<T>::Environment(map m) : map_{std::move(m)} {
  if (std::any_of(map_.begin(), map_.end(), [](const auto& p) { return p.first.is_dummy(); }))
    SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
}

template <class T>
void Environment<T>::insert(const key_type& key, const T& elem) {
  if (key.is_dummy()) SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
  map_.emplace(key, elem);
}

template <class T>
void Environment<T>::insert_or_assign(const key_type& key, const T& elem) {
  if (key.is_dummy()) SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
  map_.insert_or_assign(key, elem);
}

template <class T>
Variables Environment<T>::domain() const {
  Variables dom;
  for (const auto& p : map_) dom += p.first;
  return dom;
}

template <class T>
const T& Environment<T>::at(const key_type& key) const {
  return map_.at(key);
}

template <class T>
T& Environment<T>::operator[](const key_type& key) {
  return map_[key];
}

template <class T>
const T& Environment<T>::operator[](const key_type& key) const {
  return map_.at(key);
}

template <class T>
bool Environment<T>::operator==(const Environment<T>& other) const {
  if (map_.size() != other.map_.size()) return false;
  return std::all_of(map_.begin(), map_.end(), [&other](const auto& p) {
    return other.map_.find(p.first) != other.map_.end() && other.map_.at(p.first) == p.second;
  });
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Environment<T>& env) {
  for (const auto& [var, value] : env) os << var << " -> " << value << ", ";
  return os;
}

template class Environment<int>;
template class Environment<long>;
template class Environment<float>;
template class Environment<double>;

template std::ostream& operator<<(std::ostream& os, const Environment<int>& env);
template std::ostream& operator<<(std::ostream& os, const Environment<long>& env);
template std::ostream& operator<<(std::ostream& os, const Environment<float>& env);
template std::ostream& operator<<(std::ostream& os, const Environment<double>& env);

}  // namespace smats
