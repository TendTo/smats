/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @copyright 2019 Drake (https://drake.mit.edu)
 * @licence Apache-2.0 license
 * @brief Variable class
 *
 * Represents a symbolic variable.
 * A symbolic variable is a named entity that can take a value from a specific domain.
 */
#pragma once
// IWYU pragma: no_include <string_view>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "smats/util/concepts.h"
#include "smats/util/hash.hpp"

namespace smats {

/**
 * Represents a symbolic variable.
 *
 * A symbolic variable is a named entity that can take a value from a specific domain.
 * @warning Do not use the @p operator== to verify equality between to Variable, as it creates a Formula.
 * Use the @ref equal_to() method instead.
 */
class Variable {
 public:
  using Id = std::size_t;

  /** Supported types of symbolic variables. */
  enum class Type : std::uint8_t {
    CONTINUOUS,  ///< A CONTINUOUS variable takes a `double` value.
    INTEGER,     ///< An INTEGER variable takes an `int` value.
    BINARY,      ///< A BINARY variable takes an integer value from {0, 1}.
    BOOLEAN,     ///< A BOOLEAN variable takes a `bool` value.
  };

  /**
   * Construct a dummy variable.
   *
   * All default-constructed variables are considered the same variable by the @ref equal_to method.
   * A moved-from variable becomes a dummy variable.
   */
  Variable() : id_{0}, name_{nullptr} {};
  /**
   * Constructs a variable with a string.
   * @param name name of the variable.
   * @param type type of the variable.
   */
  explicit Variable(const std::string& name, Type type = Type::CONTINUOUS);
  Variable(const Variable&) = default;
  Variable(Variable&& other) noexcept : id_(other.id_), name_(std::move(other.name_)) { other.id_ = 0; }
  Variable& operator=(const Variable&) = default;
  Variable& operator=(Variable&& other) noexcept {
    id_ = other.id_;
    name_ = std::move(other.name_);
    other.id_ = 0;
    return *this;
  }

  /**
   * @checker{a dummy variable, variable,
   * A dummy variable is a variable with an ID of zero and represents an anonymous variable.
   * It should not be used in any context other than as a placeholder.}
   */
  [[nodiscard]] bool is_dummy() const { return id_ == 0; }
  /** @getter{unique identifier, variable}*/
  [[nodiscard]] Id id() const { return id_; }
  /** @getter{type, variable, The type is stored in the upper byte of @ref id_ ., get_next_id()}*/
  [[nodiscard]] Type type() const { return static_cast<Type>(Id{id_} >> (7 * 8)); }
  /** @getter{name, variable} */
  [[nodiscard]] const std::string& name() const;

  /** @equal_to{variables, Two variables are equal if they have the same @ref id_ .} */
  [[nodiscard]] inline bool equal_to(const Variable& o) const noexcept { return id_ == o.id_; }
  /** @less{variables, The ordering is based on the @ref id_ values of the two variables.} */
  [[nodiscard]] inline bool less(const Variable& o) const noexcept { return id_ < o.id_; }
  /** @hash{variable} */
  inline void hash(InvocableHashAlgorithm auto& hasher) const noexcept { hash_append(hasher, id_); }

 private:
  /**
   * Get the next unique identifier for a variable.
   *
   * The unique identifier is a 64-bit value that is composed of two parts:
   * - The first high-order byte stores the @ref Type of the variable
   * - The remaining low-order bytes store a counter that is incremented each time a new variable is created.
   *
   * @note Id 0 is reserved for anonymous variable which is created by the default constructor, @ref Variable().
   * As a result, the invariant `get_next_id() > 0` is guaranteed.
   * @param type type of the variable
   * @return next unique identifier for a variable
   */
  static Id get_next_id(Variable::Type type);

  Id id_;  ///< Unique identifier for the variable. The high-order byte stores the Type. @see get_next_id()
  std::shared_ptr<const std::string> name_;  ///< Name of the variable.
};

std::ostream& operator<<(std::ostream& os, const Variable& var);

std::ostream& operator<<(std::ostream& os, Variable::Type type);

}  // namespace smats

namespace std {
template <>
struct less<smats::Variable> {
  bool operator()(const smats::Variable& lhs, const smats::Variable& rhs) const { return lhs.less(rhs); }
};

template <>
struct equal_to<smats::Variable> {
  bool operator()(const smats::Variable& lhs, const smats::Variable& rhs) const { return lhs.equal_to(rhs); }
};

template <>
struct hash<smats::Variable> : public smats::DefaultHash {};

}  // namespace std
