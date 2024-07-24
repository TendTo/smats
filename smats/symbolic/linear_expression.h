/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Expression class.
 */
#pragma once

#include <algorithm>  // for cpplint only
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <numbers>
#include <ostream>
#include <random>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "smats/symbolic/environment.h"
#include "smats/symbolic/expression_kind.h"
#include "smats/symbolic/variable.h"
#include "smats/symbolic/variables.h"
#include "smats/util/definitions.h"
#include "smats/util/hash.hpp"

namespace smats {

/**
* Represents a symbolic form of a linear or piecewise linear expression.

Its syntax tree is as follows:

@verbatim
E := Var | Constant | E + ... + E | abs(E) | min(E, E) | max(E, E) | ceil(E) | floor(E) | if_then_else(F, E, E)
@endverbatim

In the implementation, Expression stores a shared pointer to a const ExpressionCell class
that is a super-class of different kinds of symbolic expressions
(i.e., ExpressionAdd, ExpressionVar, ExpressionMax), which makes it efficient to copy,
move, and assign to an Expression.

@note -E is represented as -1 * E internally.

@note A subtraction E1 - E2 is represented as E1 + (-1 * E2) internally.

The following simple simplifications are implemented:
@verbatim
E + 0             ->  E
0 + E             ->  E
E - 0             ->  E
E - E             ->  0
E * 1             ->  E
1 * E             ->  E
E * 0             ->  0
0 * E             ->  0
E / 1             ->  E
E / E             ->  1
pow(E, 0)         ->  1
pow(E, 1)         ->  E
E * E             ->  E^2 (= pow(E, 2))
sqrt(E * E)       ->  |E| (= abs(E))
sqrt(E) * sqrt(E) -> E
@endverbatim

Constant folding is implemented:
@verbatim
E(c1) + E(c2)  ->  E(c1 + c2)    // c1, c2 are constants
E(c1) - E(c2)  ->  E(c1 - c2)
E(c1) * E(c2)  ->  E(c1 * c2)
E(c1) / E(c2)  ->  E(c1 / c2)
f(E(c))        ->  E(f(c))       // c is a constant, f is a math function
@endverbatim

For the math functions which are only defined over a restricted domain (namely,
log, sqrt, pow, asin, acos), we check the domain of argument(s), and throw
std::domain_error exception if a function is not well-defined for a given
argument(s).

Relational operators over expressions (==, !=, <, >, <=, >=) return
symbolic::Formula instead of bool. Those operations are declared in formula.h
file. To check structural equality between two expressions a separate function,
Expression::EqualTo, is provided.

Regarding the arithmetic of an Expression when operating on NaNs, we have the
following rules:
1. NaN values are extremely rare during typical computations. Because they are
difficult to handle symbolically, we will round that up to "must never
occur". We allow the user to form ExpressionNaN cells in a symbolic
tree. For example, the user can initialize an Expression to NaN and then
overwrite it later. However, evaluating a tree that has NaN in its evaluated
sub-trees is an error (see rule (3) below).
2. It's still valid for code to check `isnan` in order to fail-fast. So we
provide isnan(const Expression&) for the common case of non-NaN value
returning False. This way, code can fail-fast with mpq_class yet still compile
with Expression.
3. If there are expressions that embed separate cases (`if_then_else`), some of
the sub-expressions may be not used in evaluation when they are in the
not-taken case (for NaN reasons or any other reason). Bad values within
those not-taken branches does not cause exceptions.
4. The isnan check is different than if_then_else. In the latter, the
ExpressionNaN is within a dead sub-expression branch. In the former, it
appears in an evaluated trunk. That goes against rule (1) where a NaN
anywhere in a computation (other than dead code) is an error.

@internal note for Drake developers: under the hood of Expression, we have an
internal::BoxedCell helper class that uses NaN for pointer tagging; that's a
distinct concept from the Expression::NaN() rules enumerated just above.

symbolic::Expression can be used as a scalar type of Eigen types.
*/
template <class T>
class LinearExpression : Expression<T> {
  friend class ExpressionCell<T>;
  friend class ExpressionAddFactory<T>;

 public:
  /** @constructor{expression, Default to zero} */
  LinearExpression() = default;
  LinearExpression(const T& constant);  // NOLINT (runtime/explicit): This conversion is desirable.
  /**
   * Constructs an expression from @p var.
   * @pre @p var is not a BOOLEAN variable.
   */
  LinearExpression(const Variable& var);  // NOLINT (runtime/explicit): This conversion is desirable.

  /** @checker{polynomial, expression} */
  [[nodiscard]] bool is_polynomial() const;

  /**
   * Returns true if this symbolic expression is already
   * expanded. Expression::Expand() uses this flag to avoid calling
   * ExpressionCell::Expand() on an pre-expanded expressions.
   * Expression::Expand() also sets this flag before returning the result.
   *
   * @note This check is conservative in that `false` does not always indicate
   * that the expression is not expanded. This is because exact checks can be
   * costly and we want to avoid the exact check at the construction time.
   */
  [[nodiscard]] bool is_expanded() const;

  /**
   * Differentiates this symbolic expression with respect to the variable @p var.
   * @throws std::exception if it is not differentiable.
   */
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const;

  /** @getter{string representation, expression}. */
  [[nodiscard]] std::string to_string() const;

  /** @hash{expression} */
  void hash(InvocableHashAlgorithm auto& hasher) const noexcept { cell_->hash(hasher); }

  ARITHMETIC_OPERATORS(Expression<T>)
  Expression<T> operator-() const;
  Expression<T>& operator++();
  Expression<T> operator++(int);
  Expression<T>& operator--();
  Expression<T> operator--(int);

  // Cast functions which takes a pointer to a non-const Expression.
  bool is_constant() const;
  bool is_constant(const T& value) const;
  bool is_variable() const;
  bool is_variable(const Variable& var) const;
  bool is_addition() const;
  bool is_multiplication() const;

  const T& constant_value() const;

  // TODO(ernesto): remove
  std::shared_ptr<const ExpressionCell<T>>& get() { return cell_; }

 private:
  explicit Expression(const std::shared_ptr<const ExpressionCell<T>>& cell);

  // Returns a const reference to the owned cell.
  // @pre This expression is not a Constant.
  const ExpressionCell<T>& cell() const { return *cell_; }

  // Returns a mutable reference to the owned cell. This function may only be
  // called when this object is the sole owner of the cell (use_count == 1).
  // @pre This expression is not an ExpressionKind::Constant.
  ExpressionCell<T>& m_cell();

  std::shared_ptr<const ExpressionCell<T>> cell_;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const LinearExpression<T>& e);

EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(LinearExpression);

using LinearExpressionD = LinearExpression<double>;
using LinearExpressionF = LinearExpression<float>;
using LinearExpressionI = LinearExpression<int>;
using LinearExpressionL = LinearExpression<long>;

}  // namespace smats

namespace std {
/* Provides std::hash<smats::Expression>. */
template <class T>
struct hash<smats::LinearExpression<T>> : public smats::DefaultHash {};

/* Provides std::less<smats::Expression>. */
template <class T>
struct less<smats::LinearExpression<T>> {
  bool operator()(const smats::LinearExpression<T>& lhs, const smats::LinearExpression<T>& rhs) const {
    return lhs.less(rhs);
  }
};

/* Provides std::equal_to<smats::Expression>. */
template <class T>
struct equal_to<smats::LinearExpression<T>> {
  bool operator()(const smats::LinearExpression<T>& lhs, const smats::LinearExpression<T>& rhs) const {
    return lhs.equal_to(rhs);
  }
};

}  // namespace std
