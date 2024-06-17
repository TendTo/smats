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
template <class T>
typename Environment<T>::map BuildMap(const std::initializer_list<typename Environment<T>::key_type> vars) {
  typename Environment<T>::map m;
  for (const typename Environment<T>::key_type& var : vars) m.emplace(var, 0.0);
  return m;
}

}  // namespace

template <class T>
Environment<T>::Environment(const std::initializer_list<value_type> init) : Environment<T>{map(init)} {}

template <class T>
Environment<T>::Environment(const std::initializer_list<key_type> vars) : Environment{BuildMap<T>(vars)} {}

template <class T>
Environment<T>::Environment(map m) : map_{std::move(m)} {
  if (std::any_of(map_.begin(), map_.end(), [](const auto& p) { return p.first.is_dummy(); }))
    SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
}

template <class T>
void Environment<T>::insert(const key_type& key, const mapped_type& elem) {
  if (key.is_dummy()) SMATS_RUNTIME_ERROR("Cannot insert dummy variable into Environment");
  map_.emplace(key, elem);
}

template <class T>
void Environment<T>::insert_or_assign(const key_type& key, const mapped_type& elem) {
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
const Environment<T>::mapped_type& Environment<T>::at(const key_type& key) const {
  return map_.at(key);
}

template <class T>
Environment<T>::mapped_type& Environment<T>::operator[](const key_type& key) {
  return map_[key];
}

template <class T>
const Environment<T>::mapped_type& Environment<T>::operator[](const key_type& key) const {
  return map_.at(key);
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
template class Environment<mpq_class>;

template std::ostream& operator<<(std::ostream& os, const Environment<int>& env);
template std::ostream& operator<<(std::ostream& os, const Environment<long>& env);
template std::ostream& operator<<(std::ostream& os, const Environment<float>& env);
template std::ostream& operator<<(std::ostream& os, const Environment<double>& env);
template std::ostream& operator<<(std::ostream& os, const Environment<mpq_class>& env);

}  // namespace smats
