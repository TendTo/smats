/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#pragma once
// IWYU pragma: no_include <string_view>

#include <initializer_list>
#include <ostream>
#include <set>

#include "smats/symbolic/variable.h"
#include "smats/util/concepts.h"
#include "smats/util/hash.hpp"

namespace smats {

/** Represents a set of variables.
 *
 * This class is based on std::set<Variable>. The intent is to add things that
 * we need including set-union (Variables::insert, operator+, operator+=),
 * set-minus (Variables::erase, operator-, operator-=), and subset/superset
 * checking functions (Variables::IsSubsetOf, Variables::IsSupersetOf,
 * Variables::IsStrictSubsetOf, Variables::IsStrictSupersetOf).
 */
class Variables {
 public:
  using size_type = std::set<Variable>::size_type;
  using iterator = std::set<Variable>::iterator;
  using const_iterator = std::set<Variable>::const_iterator;
  using reverse_iterator = std::set<Variable>::reverse_iterator;
  using const_reverse_iterator = std::set<Variable>::const_reverse_iterator;

  /** @constructor{Variables} */
  Variables() = default;
  /**
   * Construct a new Variables object initialized with a list of variables.
   * @param init initializer list of variables
   */
  Variables(std::initializer_list<Variable> init);

  /** @getter{number of variables, set} */
  [[nodiscard]] size_type size() const { return vars_.size(); }
  /** @checker{empty, set} */
  [[nodiscard]] bool empty() const { return vars_.empty(); }
  /** @hash{variables} */
  void hash(InvocableHashAlgorithm auto& hasher) const noexcept { hash_append(hasher, vars_); }
  /** @getter{iterator to the beginning, set} */
  iterator begin() { return vars_.begin(); }
  /** @getter{iterator to the end, set} */
  iterator end() { return vars_.end(); }
  /** @getter{const iterator to the beginning, set} */
  [[nodiscard]] const_iterator begin() const { return vars_.cbegin(); }
  /** @getter{const iterator to the end, set} */
  [[nodiscard]] const_iterator end() const { return vars_.cend(); }
  /** @getter{const iterator to the beginning, set} */
  [[nodiscard]] const_iterator cbegin() const { return vars_.cbegin(); }
  /** @getter{const iterator to the end, set} */
  [[nodiscard]] const_iterator cend() const { return vars_.cend(); }
  /** @getter{reverse iterator to the beginning, set} */
  reverse_iterator rbegin() { return vars_.rbegin(); }
  /** @getter{reverse iterator to the end, set} */
  reverse_iterator rend() { return vars_.rend(); }
  /** @getter{reverse const iterator to the beginning, set} */
  [[nodiscard]] const_reverse_iterator rbegin() const { return vars_.crbegin(); }
  /** @getter{reverse const iterator to the end, set} */
  [[nodiscard]] const_reverse_iterator rend() const { return vars_.crend(); }
  /** @getter{reverse const iterator to the beginning, set} */
  [[nodiscard]] const_reverse_iterator crbegin() const { return vars_.crbegin(); }
  /** @getter{reverse const iterator to the end, set} */
  [[nodiscard]] const_reverse_iterator crend() const { return vars_.crend(); }

  /**
   * Inserts a variable @p var into a set.
   * @param var variable to insert
   */
  void insert(const Variable& var) { vars_.insert(var); }
  /**
   * Inserts variables in the range [first, last) into a set.
   * @tparam InputIt type of the iterator
   * @param first iterator to the beginning
   */
  template <class InputIt>
  void insert(InputIt first, InputIt last) {
    vars_.insert(first, last);
  }
  /**
   * Inserts variables in @p vars into a set.
   * @param vars set of variables
   */
  void insert(const Variables& vars) { vars_.insert(vars.begin(), vars.end()); }

  /**
   * Erases @p key from a set.
   * Return number of erased elements (0 or 1).
   * @param key variable to erase
   * @return number of erased elements
   */
  size_type erase(const Variable& key) { return vars_.erase(key); }

  /**
   * Erases variables in @p vars from a set.
   * Return number of erased elements ([0, vars.size()]).
   * @param vars set of variables
   * @return number of erased elements
   */
  size_type erase(const Variables& vars);

  /**
   * Finds element with specific key.
   * @param key variable to find
   * @return iterator to the element with key equivalent to @p key
   */
  iterator find(const Variable& key) { return vars_.find(key); }
  /**
   * Finds element with specific key.
   * @param key variable to find
   * @return const_iterator to the element with key equivalent to @p key
   */
  [[nodiscard]] const_iterator find(const Variable& key) const { return vars_.find(key); }
  /**
   * @checker{contained in the set, variable @p key }
   * @param key variable to check
   */
  [[nodiscard]] bool contains(const Variable& key) const { return find(key) != end(); }

  /**
   * @checker{subset of @p vars, this set of variables}
   * @param vars set of variables
   */
  [[nodiscard]] bool is_subset_of(const Variables& vars) const;
  /**
   * @checker{superset of @p vars, this set of variables}
   * @param vars set of variables
   */
  [[nodiscard]] bool is_superset_of(const Variables& vars) const;
  /**
   * @checker{strict subset of @p vars, this set of variables}
   * @param vars set of variables
   */
  [[nodiscard]] bool is_strict_subset_of(const Variables& vars) const;
  /**
   * @checker{strict superset of @p vars, this set of variables}
   * @param vars set of variables
   */
  [[nodiscard]] bool is_strict_superset_of(const Variables& vars) const;

  bool operator==(const Variables& vars) const;
  bool operator<(const Variables& vars) const;

  Variables& operator+=(const Variables& vars);
  Variables& operator+=(const Variable& var);

  Variables& operator-=(const Variables& vars);
  Variables& operator-=(const Variable& var);

  Variables operator+(const Variables& vars) const;
  Variables operator+(const Variable& var) const;

  Variables operator-(const Variables& vars) const;
  Variables operator-(const Variable& var) const;

  /**
   * Return the intersection of two sets of variables.
   * @param vars set of variables
   * @return intersection of two sets of variables
   */
  [[nodiscard]] Variables intersect(const Variables& vars) const;

 private:
  /**
   * Construct a new Variables object from a set of variables.
   * @param vars set of variables
   */
  explicit Variables(std::set<Variable> vars);

  std::set<Variable> vars_;  ///< Set of variables
};

Variables operator+(const Variable& var, Variables vars);

std::ostream& operator<<(std::ostream&, const Variables& vars);

}  // namespace smats

namespace std {
template <>
struct hash<smats::Variables> : public smats::DefaultHash {};
}  // namespace std
