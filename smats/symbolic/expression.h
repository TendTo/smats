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

template <class T>
class ExpressionCell;  // In expression_cell.h
template <class T>
class ExpressionVar;  // In expression_cell.h
template <class T>
class UnaryExpressionCell;  // In expression_cell.h
template <class T>
class BinaryExpressionCell;  // In expression_cell.h
template <class T>
class ExpressionAdd;  // In expression_cell.h
template <class T>
class ExpressionMul;  // In expression_cell.h
template <class T>
class ExpressionDiv;  // In expression_cell.h
template <class T>
class ExpressionLog;  // In expression_cell.h
template <class T>
class ExpressionAbs;  // In expression_cell.h
template <class T>
class ExpressionExp;  // In expression_cell.h
template <class T>
class ExpressionSqrt;  // In expression_cell.h
template <class T>
class ExpressionPow;  // In expression_cell.h
template <class T>
class ExpressionSin;  // In expression_cell.h
template <class T>
class ExpressionCos;  // In expression_cell.h
template <class T>
class ExpressionTan;  // In expression_cell.h
template <class T>
class ExpressionAsin;  // In expression_cell.h
template <class T>
class ExpressionAcos;  // In expression_cell.h
template <class T>
class ExpressionAtan;  // In expression_cell.h
template <class T>
class ExpressionAtan2;  // In expression_cell.h
template <class T>
class ExpressionSinh;  // In expression_cell.h
template <class T>
class ExpressionCosh;  // In expression_cell.h
template <class T>
class ExpressionTanh;  // In expression_cell.h
template <class T>
class ExpressionMin;  // In expression_cell.h
template <class T>
class ExpressionMax;  // In expression_cell.h
template <class T>
class ExpressionCeiling;  // In expression_cell.h
template <class T>
class ExpressionFloor;  // In expression_cell.h
template <class T>
class ExpressionIfThenElse;  // In expression_cell.h
template <class T>
class ExpressionUninterpretedFunction;  // In expression_cell.h
template <class T>
class Formula;  // In formula.h
template <class T>
class ExpressionAddFactory;  // In expression_factory.h
template <class T>
class ExpressionMulFactory;  // In expression_factory.h

/**
* Represents a symbolic form of an expression.

Its syntax tree is as follows:

@verbatim
E := Var | Constant | E + ... + E | E * ... * E | E / E | log(E)
   | abs(E) | exp(E) | sqrt(E) | pow(E, E) | sin(E) | cos(E) | tan(E)
   | asin(E) | acos(E) | atan(E) | atan2(E, E) | sinh(E) | cosh(E) | tanh(E)
   | min(E, E) | max(E, E) | ceil(E) | floor(E) | if_then_else(F, E, E)
   | NaN | uninterpreted_function(name, {v_1, ..., v_n})
@endverbatim

In the implementation, Expression directly stores a shared pointer to a const ExpressionCell class
that is a super-class of different kinds of symbolic expressions
(i.e., ExpressionAdd, ExpressionMul, ExpressionLog, ExpressionSin), which makes it efficient to copy,
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
class Expression {
  friend class ExpressionCell<T>;
  friend class ExpressionAddFactory<T>;
  friend class ExpressionMulFactory<T>;

 public:
  static Expression<T> zero();
  static Expression<T> one();
  static Expression<T> pi() {
    static Expression pi{static_cast<T>(std::numbers::pi)};
    return pi;
  }
  static Expression<T> e() {
    static Expression e{static_cast<T>(std::numbers::e)};
    return e;
  }
  static Expression<T> NaN();

  /** @constructor{expression, Default to zero} */
  Expression();
  Expression(const T& constant);  // NOLINT (runtime/explicit): This conversion is desirable.
  /**
   * Constructs an expression from @p var.
   * @pre @p var is not a BOOLEAN variable.
   */
  Expression(const Variable& var);  // NOLINT (runtime/explicit): This conversion is desirable.

  /** @getter{kind, expression} */
  [[nodiscard]] ExpressionKind kind() const;

  /** @getter{variables, expression} */
  [[nodiscard]] Variables variables() const;

  /**
   * Checks structural equality.
   *
   * Two expressions e1 and e2 are structurally equal when they have the same
   * internal AST(abstract-syntax tree) representation. Please note that we can
   * have two computationally (or extensionally) equivalent expressions which
   * are not structurally equal. For example, consider:
   * @f[
   *    e1 = 2 * (x + y) \\
   *    e2 = 2x + 2y
   * @f]
   * Obviously, we know that e1 and e2 are evaluated to the same value for all
   * assignments to x and y. However, e1 and e2 are not structurally equal by
   * the definition. Note that e1 is a multiplication expression
   * (is_multiplication(e1) is true) while e2 is an addition expression
   * (is_addition(e2) is true).
   *
   * One main reason we use structural equality in EqualTo is due to
   * Richardson's Theorem. It states that checking ∀x. E(x) = F(x) is
   * undecidable when we allow sin, asin, log, exp in E and F. Read
   * https://en.wikipedia.org/wiki/Richardson%27s_theorem for details.
   *
   * Note that for polynomial cases, you can use Expand method and check if two
   * polynomial expressions p1 and p2 are computationally equal. To do so, you
   * check the following:
   * @code
   *     p1.Expand().EqualTo(p2.Expand())
   * @endcode
   */
  [[nodiscard]] bool equal_to(const Expression<T>& e) const;

  /** @less{expressions} */
  [[nodiscard]] bool less(const Expression<T>& e) const;

  /** @checker{polynomial, expression} */
  [[nodiscard]] bool is_polynomial() const;

  /**
   * Evaluates using a given environment (by default, an empty environment).
   * @throws std::exception if there exists a non-random variable in this expression
   * whose assignment is not provided by @p env
   * @throws std::exception if an unassigned random variable is detected while @p random_generator is `nullptr`
   * @throws std::exception if NaN is detected during evaluation.
   */
  [[nodiscard]] T evaluate(const Environment<T>& env = {}) const;

  /**
   * Partially evaluates this expression using an environment @p env.
   *
   * Internally, this method uses @p env to substitute (Variable → Constant) and call Evaluate::Substitute with it.
   * @throws std::exception if NaN is detected during evaluation.
   */
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const;

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
   * Expands out products and positive integer powers in expression.
   * For example, @f$(x + 1) \cdot (x - 1)@f$ is expanded to @f$x^2 - 1$@f
   * and @f$(x + y)^2@f$ is expanded to @f$x^2 + 2xy + y^2@f$.
   * Note that Expand applies recursively to sub-expressions.
   * For instance, @f$\sin(2 * (x + y))@f$ is expanded to @f$\sin(2x + 2y)@f$.
   * It also simplifies "division by constant" cases.
   * @throws std::exception if NaN is detected during expansion.
   */
  [[nodiscard]] Expression<T> expand() const;

  /**
   * Returns a copy of this expression replacing all occurrences of @p var with @p e.
   * @throws std::exception if NaN is detected during substitution.
   */
  [[nodiscard]] Expression<T> substitute(const Variable& var, const Expression<T>& e) const;

  /**
   * Returns a copy of this expression,
   * replacing all occurrences of the variables in @p s with corresponding expressions in @p s.
   *
   * Note that the substitutions occur simultaneously.
   * @code
   * Expression e = x / y;
   * Substitution s = {{x, y}, {y, x}};
   * e.Substitute(s);  // returns y / x
   * @endcode
   * @throws std::exception if NaN is detected during substitution.
   */
  [[nodiscard]] Expression<T> substitute(const std::unordered_map<Variable, Expression<T>>& s) const;

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
  GENERIC_ARITHMETIC_OPERATORS(Expression<T>, T&)
  GENERIC_ARITHMETIC_OPERATORS(Expression<T>, Variable&)
  Expression<T> operator-() const;
  Expression<T>& operator++();
  Expression<T> operator++(int);
  Expression<T>& operator--();
  Expression<T> operator--(int);
  Expression<T>& operator^=(const Expression<T>& o);
  Expression<T> operator^(const Expression<T>& o) const;

  // Cast functions which takes a pointer to a non-const Expression.
  [[nodiscard]] bool is_leaf() const;
  [[nodiscard]] bool is_constant() const;
  [[nodiscard]] bool is_constant(const T& value) const;
  [[nodiscard]] bool is_variable() const;
  [[nodiscard]] bool is_variable(const Variable& var) const;
  [[nodiscard]] bool is_addition() const;
  [[nodiscard]] bool is_multiplication() const;
  [[nodiscard]] bool is_division() const;
  [[nodiscard]] bool is_nan() const;
  [[nodiscard]] bool is_pow() const;
  template <ExpressionKind K>
  [[nodiscard]] bool is() const {
    return cell_->kind() == K;
  }

  [[nodiscard]] const T& constant() const;
  [[nodiscard]] const std::map<Expression<T>, T>& expression_to_coeff_map() const;
  [[nodiscard]] const std::map<Expression<T>, Expression<T>>& base_to_exponent_map() const;
  [[nodiscard]] const Expression<T>& lhs() const;
  [[nodiscard]] const Expression<T>& rhs() const;

  /** @getter{reference count, underlying expression cell} */
  [[nodiscard]] long use_count() const { return cell_.use_count(); }

 private:
  explicit Expression(const std::shared_ptr<const ExpressionCell<T>>& cell, bool is_expanded = false);

  /** @getter{underlying expression cell, expression} */
  [[nodiscard]] const ExpressionCell<T>& cell() const { return *cell_; }
  /**
   * @getsetter{underlying expression cell, expression,
   * This function may only be called when this object is the sole owner of the cell (use_count == 1)
   * @pre This expression is the sole owner of the cell (use_count == 1)}
   */
  [[nodiscard]] ExpressionCell<T>& m_cell();

  std::shared_ptr<const ExpressionCell<T>> cell_;
};

template <class T>
Expression<T> operator+(const T& c, const Expression<T>& e);
template <class T>
Expression<T> operator-(const T& c, const Expression<T>& e);
template <class T>
Expression<T> operator*(const T& c, const Expression<T>& e);
template <class T>
Expression<T> operator/(const T& c, const Expression<T>& e);
template <class T>
Expression<T> operator^(const T& c, const Expression<T>& e);

template <class T>
std::ostream& operator<<(std::ostream& os, const Expression<T>& e);

EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(Expression);

using ExpressionD = Expression<double>;
using ExpressionF = Expression<float>;
using ExpressionI = Expression<int>;
using ExpressionL = Expression<long>;

}  // namespace smats

namespace std {

/* Provides std::hash<smats::Expression>. */
template <class T>
struct hash<smats::Expression<T>> : public smats::DefaultHash {};

/* Provides std::less<smats::Expression>. */
template <class T>
struct less<smats::Expression<T>> {
  bool operator()(const smats::Expression<T>& lhs, const smats::Expression<T>& rhs) const { return lhs.less(rhs); }
};

/* Provides std::equal_to<smats::Expression>. */
template <class T>
struct equal_to<smats::Expression<T>> {
  bool operator()(const smats::Expression<T>& lhs, const smats::Expression<T>& rhs) const { return lhs.equal_to(rhs); }
};

}  // namespace std
