/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Environment class
 *
 * Represents a symbolic environment (mapping from a variable to a value).
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <ostream>
#include <span>
#include <unordered_map>

#include "smats/symbolic/variable.h"
#include "smats/symbolic/variables.h"

namespace smats {
/** Represents a symbolic environment (mapping from a variable to a value).
 *
 * This class is used when we evaluate symbolic expressions or formulas which
 * include unquantified (free) variables. Here are examples:
 * @code
 * const Variable var_x{"x"};
 * const Variable var_y{"y"};
 * const Expression x{var_x};
 * const Expression y{var_x};
 * const Expression e1{x + y};
 * const Expression e2{x - y};
 * const Formula f{e1 > e2};
 *
 * // env maps var_x to 2.0 and var_y to 3.0
 * const Environment env{{var_x, 2.0}, {var_y, 3.0}};
 *
 * const numeric_type res1 = e1.Evaluate(env);  // x + y => 2.0 + 3.0 =>  5.0
 * const numeric_type res2 = e2.Evaluate(env);  // x - y => 2.0 - 3.0 => -1.0
 * const bool res = f.Evaluate(env);  // x + y > x - y => 5.0 >= -1.0 => True
 * @endcode
 * @tparam T type of the value
 */
template <class T>
class Environment {
 public:
  using key_type = Variable;
  using mapped_type = T;
  using map = std::unordered_map<key_type, T>;
  using value_type = map::value_type;
  using iterator = map::iterator;
  using const_iterator = map::const_iterator;

  /** @constructor{environment} */
  Environment() = default;

  /**
   * Construct a new environment object from a list of (Variable, numeric_type).
   * @param init initializer list of (Variable, numeric_type)
   * @throws std::runtime_exception if @p init include a dummy variable or a NaN value.
   */
  explicit Environment(std::initializer_list<value_type> init);

  /**
   * Construct a new environment object from a list of (Variable, numeric_type).
   * @param init initializer list of (Variable, numeric_type)
   * @throws std::runtime_exception if @p init include a dummy variable or a NaN value.
   */
  explicit Environment(std::span<const value_type> init);

  /**
   * Construct a new environment object from a list of variables.
   *
   * Initializes the variables with 0.0.
   * @param vars initializer list of variables
   * @throws std::runtime_exception if @p vars include a dummy variable.
   */
  explicit Environment(std::initializer_list<key_type> vars);

  /**
   * Construct a new environment object from a list of variables.
   *
   * Initializes the variables with 0.0.
   * @param vars initializer list of variables
   * @throws std::runtime_exception if @p vars include a dummy variable.
   */
  explicit Environment(std::span<const key_type> vars);

  /**
   * Construct a new environment object from the map @p m between variables and values.
   * @param m map between variables and values
   * @throws std::runtime_exception if @p m include a dummy variable or a NaN value.
   */
  explicit Environment(map m);

  /** @getter{iterator to the beginning, environment} */
  iterator begin() { return map_.begin(); }
  /** @getter{iterator to the end, environment} */
  iterator end() { return map_.end(); }
  /** @getter{const iterator to the beginning, environment} */
  [[nodiscard]] const_iterator begin() const { return map_.cbegin(); }
  /** @getter{const iterator to the end, environment} */
  [[nodiscard]] const_iterator end() const { return map_.cend(); }
  /** @getter{const iterator to the beginning, environment} */
  [[nodiscard]] const_iterator cbegin() const { return map_.cbegin(); }
  /** @getter{const iterator to the end, environment} */
  [[nodiscard]] const_iterator cend() const { return map_.cend(); }

  /**
   * Insert a pair (@p key, @p elem) if this environment doesn't contain @p key.
   *
   * Similar to insert function in map,
   * if the key already exists in this environment, then calling insert(key, elem) doesn't change
   * the existing key-value in this environment.
   * @param key key to insert
   * @param elem value to insert
   */
  void insert(const key_type& key, const T& elem);
  /**
   * Insert a pair (@p key, @p elem) or assign @p elem to the existing key if it exists.
   * @param key key to insert
   * @param elem value to insert
   */
  void insert_or_assign(const key_type& key, const T& elem);

  /** @checker{empty, environment} */
  [[nodiscard]] bool empty() const { return map_.empty(); }
  /** @getter{number of elements, environment} */
  [[nodiscard]] std::size_t size() const { return map_.size(); }
  /** @getter{domain, environment} */
  [[nodiscard]] Variables domain() const;

  /** Check if the environment contains the @p key.
   * @param key key to check
   * @return true if the environment contains @p key
   * @return false if the environment doesn't contain @p key
   */
  [[nodiscard]] bool contains(const key_type& key) const { return map_.contains(key); }
  /**
   * Finds element with specific key.
   * @param key key to find
   * @return iterator to the element with key equivalent to @p key.
   */
  iterator find(const key_type& key) { return map_.find(key); }
  /**
   * Finds element with specific key.
   * @param key key to find
   * @return const_iterator to the element with key equivalent to @p key.
   */
  [[nodiscard]] const_iterator find(const key_type& key) const { return map_.find(key); }

  /**
   * Get a reference to the value that is mapped to a key equivalent to @p key.
   * @param key key of the element to find
   * @return reference to the mapped value of the requested key
   * @throws std::out_of_range if the key does not exist
   */
  [[nodiscard]] const T& at(const key_type& key) const;

  /**
   * Returns a reference to the value that is mapped to a key equivalent to @p key,
   * performing an insertion if such key does not already exist.
   * @param key key of the element to find or insert
   */
  T& operator[](const key_type& key);

  /**
   * Returns a constant reference to the value that is mapped to a key equivalent to @p key.
   * @param key key of the element to find
   * @return reference to the mapped value of the requested key
   * @throws std::out_of_range if the key does not exist
   */
  const T& operator[](const key_type& key) const;

  bool operator==(const Environment<T>& other) const;

 private:
  map map_;  ///< map between variables and values
};

template <class T>
std::ostream& operator<<(std::ostream& os, const Environment<T>& env);

using EnvironmentI = Environment<int>;
using EnvironmentL = Environment<long>;
using EnvironmentF = Environment<float>;
using EnvironmentD = Environment<double>;

extern template class Environment<int>;
extern template class Environment<long>;
extern template class Environment<float>;
extern template class Environment<double>;

}  // namespace smats
