#pragma once

#include <algorithm>  // for cpplint only
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <memory>
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
#include "smats/util/hash.hpp"

namespace smats {

class ExpressionCell;                   // In expression_cell.h
class ExpressionVar;                    // In expression_cell.h
class UnaryExpressionCell;              // In expression_cell.h
class BinaryExpressionCell;             // In expression_cell.h
class ExpressionAdd;                    // In expression_cell.h
class ExpressionMul;                    // In expression_cell.h
class ExpressionDiv;                    // In expression_cell.h
class ExpressionLog;                    // In expression_cell.h
class ExpressionAbs;                    // In expression_cell.h
class ExpressionExp;                    // In expression_cell.h
class ExpressionSqrt;                   // In expression_cell.h
class ExpressionPow;                    // In expression_cell.h
class ExpressionSin;                    // In expression_cell.h
class ExpressionCos;                    // In expression_cell.h
class ExpressionTan;                    // In expression_cell.h
class ExpressionAsin;                   // In expression_cell.h
class ExpressionAcos;                   // In expression_cell.h
class ExpressionAtan;                   // In expression_cell.h
class ExpressionAtan2;                  // In expression_cell.h
class ExpressionSinh;                   // In expression_cell.h
class ExpressionCosh;                   // In expression_cell.h
class ExpressionTanh;                   // In expression_cell.h
class ExpressionMin;                    // In expression_cell.h
class ExpressionMax;                    // In expression_cell.h
class ExpressionCeiling;                // In expression_cell.h
class ExpressionFloor;                  // In expression_cell.h
class ExpressionIfThenElse;             // In expression_cell.h
class ExpressionUninterpretedFunction;  // In expression_cell.h
class Formula;                          // In formula.h
class Expression;

// Substitution is a map from a Variable to a symbolic expression. It is used in
// Expression::Substitute and Formula::Substitute methods as an argument.
using Substitution = std::unordered_map<Variable, Expression>;

template <class T>
concept IsExpressionKind =
    IsAnyOf<T, ExpressionVar, UnaryExpressionCell, BinaryExpressionCell, ExpressionAdd, ExpressionMul, ExpressionDiv,
            ExpressionLog, ExpressionAbs, ExpressionExp, ExpressionSqrt, ExpressionPow, ExpressionSin, ExpressionCos,
            ExpressionTan, ExpressionAsin, ExpressionAcos, ExpressionAtan, ExpressionAtan2, ExpressionSinh,
            ExpressionCosh, ExpressionTanh, ExpressionMin, ExpressionMax, ExpressionCeiling, ExpressionFloor,
            ExpressionIfThenElse, ExpressionUninterpretedFunction>;

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

In the implementation, Expression directly stores Constant values inline, but in
all other cases stores a shared pointer to a const ExpressionCell class that is
a super-class of different kinds of symbolic expressions (i.e., ExpressionAdd,
ExpressionMul, ExpressionLog, ExpressionSin), which makes it efficient to copy,
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
class Expression {
 public:
  /** Default constructor. It constructs Zero(). */
  Expression() = default;
  Expression(const mpq_class& constant);
  Expression(int constant);

  /** Constructs an expression from @p var.
   * @pre @p var is not a BOOLEAN variable.
   */
  // NOLINTNEXTLINE(runtime/explicit): This conversion is desirable.
  Expression(const Variable& var);

  /** Returns expression kind. */
  [[nodiscard]] ExpressionKind kind() const;

  /** Collects variables in expression. */
  [[nodiscard]] Variables variables() const;

  /** Checks structural equality.
   *
   * Two expressions e1 and e2 are structurally equal when they have the same
   * internal AST(abstract-syntax tree) representation. Please note that we can
   * have two computationally (or extensionally) equivalent expressions which
   * are not structurally equal. For example, consider:
   *
   *    e1 = 2 * (x + y)
   *    e2 = 2x + 2y
   *
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
   *
   *     p1.Expand().EqualTo(p2.Expand())
   */
  [[nodiscard]] bool equal_to(const Expression& e) const;

  template <ExpressionKind kind>
  bool is_kind(const Expression& e);

  /** @less{expressions} */
  [[nodiscard]] bool less(const Expression& e) const;

  /** @checker{polynomial, expression} */
  [[nodiscard]] bool is_polynomial() const;

  /** Evaluates using a given environment (by default, an empty environment) and
   * a random number generator. If there is a random variable in this expression
   * which is unassigned in @p env, this method uses @p random_generator to
   * sample a value and use the value to substitute all occurrences of the
   * variable in this expression.
   *
   * @throws std::exception if there exists a non-random variable in this
   *                        expression whose assignment is not provided by
   *                        @p env.
   * @throws std::exception if an unassigned random variable is detected
   *                        while @p random_generator is `nullptr`.
   * @throws std::exception if NaN is detected during evaluation.
   */
  mpq_class Evaluate(const Environment& env = Environment{}) const;

  /**
   * Partially evaluates this expression using an environment @p
   * env. Internally, this method promotes @p env into a substitution
   * (Variable → Expression) and call Evaluate::Substitute with it.
   *
   * @throws std::exception if NaN is detected during evaluation.
   */
  [[nodiscard]] Expression EvaluatePartial(const Environment& env) const;

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

  /** Expands out products and positive integer powers in expression. For
   * example, `(x + 1) * (x - 1)` is expanded to `x^2 - 1` and `(x + y)^2` is
   * expanded to `x^2 + 2xy + y^2`. Note that Expand applies recursively to
   * sub-expressions. For instance, `sin(2 * (x + y))` is expanded to `sin(2x +
   * 2y)`. It also simplifies "division by constant" cases. See
   * "smats/common/test/symbolic_expansion_test.cc" to find the examples.
   *
   * @throws std::exception if NaN is detected during expansion.
   */
  [[nodiscard]] Expression Expand() const;

  /** Returns a copy of this expression replacing all occurrences of @p var
   * with @p e.
   * @throws std::exception if NaN is detected during substitution.
   */
  [[nodiscard]] Expression Substitute(const Variable& var, const Expression& e) const;

  /** Returns a copy of this expression replacing all occurrences of the
   * variables in @p s with corresponding expressions in @p s. Note that the
   * substitutions occur simultaneously. For example, (x / y).Substitute({{x,
   * y}, {y, x}}) gets (y / x).
   * @throws std::exception if NaN is detected during substitution.
   */
  [[nodiscard]] Expression Substitute(const Substitution& s) const;

  /** Differentiates this symbolic expression with respect to the variable @p
   * var.
   * @throws std::exception if it is not differentiable.
   */
  [[nodiscard]] Expression Differentiate(const Variable& x) const;

  /** Returns string representation of Expression. */
  [[nodiscard]] std::string to_string() const;

  static Expression Zero() {
    static Expression zero{0};
    return zero;
  }
  static Expression One() {
    static Expression one{1};
    return one;
  }
  static Expression Pi() {
    static Expression pi{mpq_class{M_PI}};
    return pi;
  }
  static Expression E() {
    static Expression e{mpq_class{M_E}};
    return e;
  }
  static Expression NaN();

  /** @hash{expression} */
  void hash(InvocableHashAlgorithm auto& hasher) const noexcept;

  Expression operator+(const Expression& rhs);
  // NOLINTNEXTLINE(runtime/references) per C++ standard signature.
  Expression& operator+=(const Expression& rhs);
  /** Provides prefix increment operator (i.e. ++x). */
  Expression& operator++();
  /** Provides postfix increment operator (i.e. x++). */
  Expression operator++(int);
  /** Provides unary plus operator. */
  friend Expression operator+(const Expression& e);

  friend Expression operator-(Expression lhs, const Expression& rhs);
  // NOLINTNEXTLINE(runtime/references) per C++ standard signature.
  Expression& operator-=(const Expression& rhs);

  /** Provides unary minus operator. */
  friend Expression operator-(const Expression& e);
  /** Provides prefix decrement operator (i.e. --x). */
  Expression& operator--();
  /** Provides postfix decrement operator (i.e. x--). */
  Expression operator--(int);

  friend Expression operator*(Expression lhs, const Expression& rhs) {
    lhs *= rhs;
    return lhs;
  }
  // NOLINTNEXTLINE(runtime/references) per C++ standard signature.
  friend Expression& operator*=(Expression& lhs, const Expression& rhs);
  friend Expression operator/(Expression lhs, const Expression& rhs);
  // NOLINTNEXTLINE(runtime/references) per C++ standard signature.
  friend Expression& operator/=(Expression& lhs, const Expression& rhs);

  friend Expression log(const Expression& e);
  friend Expression abs(const Expression& e);
  friend Expression exp(const Expression& e);
  friend Expression sqrt(const Expression& e);
  friend Expression pow(const Expression& e1, const Expression& e2);
  friend Expression sin(const Expression& e);
  friend Expression cos(const Expression& e);
  friend Expression tan(const Expression& e);
  friend Expression asin(const Expression& e);
  friend Expression acos(const Expression& e);
  friend Expression atan(const Expression& e);
  friend Expression atan2(const Expression& e1, const Expression& e2);
  friend Expression sinh(const Expression& e);
  friend Expression cosh(const Expression& e);
  friend Expression tanh(const Expression& e);
  friend Expression min(const Expression& e1, const Expression& e2);
  friend Expression max(const Expression& e1, const Expression& e2);
  friend Expression clamp(const Expression& v, const Expression& lo, const Expression& hi);
  friend Expression ceil(const Expression& e);
  friend Expression floor(const Expression& e);

  /** Constructs if-then-else expression.

    @verbatim
      if_then_else(cond, expr_then, expr_else)
    @endverbatim

    The value returned by the above if-then-else expression is @p expr_then if
    @p cond is evaluated to true. Otherwise, it returns @p expr_else.

    The semantics is similar to the C++'s conditional expression constructed by
    its ternary operator, @c ?:. However, there is a key difference between the
    C++'s conditional expression and our @c if_then_else expression in a way the
    arguments are evaluated during the construction.

     - In case of the C++'s conditional expression, <tt> cond ? expr_then :
       expr_else</tt>, the then expression @c expr_then (respectively, the else
       expression @c expr_else) is \b only evaluated when the conditional
       expression @c cond is evaluated to \b true (respectively, when @c cond is
       evaluated to \b false).

     - In case of the symbolic expression, <tt>if_then_else(cond, expr_then,
       expr_else)</tt>, however, \b both arguments @c expr_then and @c expr_else
       are evaluated first and then passed to the @c if_then_else function.

     @note This function returns an \b expression and it is different from the
     C++'s if-then-else \b statement.

     @note While it is still possible to define <tt> min, max, abs</tt> math
     functions using @c if_then_else expression, it is highly \b recommended to
     use the provided native definitions for them because it allows solvers to
     detect specific math functions and to have a room for special
     optimizations.

     @note More information about the C++'s conditional expression and ternary
     operator is available at
     http://en.cppreference.com/w/cpp/language/operator_other#Conditional_operator.
   */
  friend Expression if_then_else(const Formula& f_cond, const Expression& e_then, const Expression& e_else);
  friend Expression uninterpreted_function(std::string name, std::vector<Expression> arguments);

  friend std::ostream& operator<<(std::ostream& os, const Expression& e);
  friend void swap(Expression& a, Expression& b) { std::swap(a.cell_, b.cell_); }

  // Cast functions which takes a pointer to a non-const Expression.

  friend bool is_constant(const Expression& e);
  friend bool is_constant(const Expression& e, mpq_class value);
  friend bool is_nan(const Expression& e);
  friend bool is_variable(const Expression& e);
  friend bool is_addition(const Expression& e);
  friend bool is_multiplication(const Expression& e);
  friend bool is_division(const Expression& e);
  friend bool is_log(const Expression& e);
  friend bool is_abs(const Expression& e);
  friend bool is_exp(const Expression& e);
  friend bool is_sqrt(const Expression& e);
  friend bool is_pow(const Expression& e);
  friend bool is_sin(const Expression& e);
  friend bool is_cos(const Expression& e);
  friend bool is_tan(const Expression& e);
  friend bool is_asin(const Expression& e);
  friend bool is_acos(const Expression& e);
  friend bool is_atan(const Expression& e);
  friend bool is_atan2(const Expression& e);
  friend bool is_sinh(const Expression& e);
  friend bool is_cosh(const Expression& e);
  friend bool is_tanh(const Expression& e);
  friend bool is_min(const Expression& e);
  friend bool is_max(const Expression& e);
  friend bool is_ceil(const Expression& e);
  friend bool is_floor(const Expression& e);
  friend bool is_if_then_else(const Expression& e);
  friend bool is_uninterpreted_function(const Expression& e);
  friend mpq_class get_constant_value(const Expression& e);

  // Cast functions which takes a pointer to a non-const Expression.
  template <IsExpressionKind kind>
  friend kind to(const Expression& e);

  // Note that the following cast functions are only for low-level operations
  // and not exposed to the user of smats/common/symbolic/expression.h header.
  // These functions are declared in the expression_cell.h header.
  friend const ExpressionVar& to_variable(const Expression& e);
  friend const UnaryExpressionCell& to_unary(const Expression& e);
  friend const BinaryExpressionCell& to_binary(const Expression& e);
  friend const ExpressionAdd& to_addition(const Expression& e);
  friend const ExpressionMul& to_multiplication(const Expression& e);
  friend const ExpressionDiv& to_division(const Expression& e);
  friend const ExpressionLog& to_log(const Expression& e);
  friend const ExpressionAbs& to_abs(const Expression& e);
  friend const ExpressionExp& to_exp(const Expression& e);
  friend const ExpressionSqrt& to_sqrt(const Expression& e);
  friend const ExpressionPow& to_pow(const Expression& e);
  friend const ExpressionSin& to_sin(const Expression& e);
  friend const ExpressionCos& to_cos(const Expression& e);
  friend const ExpressionTan& to_tan(const Expression& e);
  friend const ExpressionAsin& to_asin(const Expression& e);
  friend const ExpressionAcos& to_acos(const Expression& e);
  friend const ExpressionAtan& to_atan(const Expression& e);
  friend const ExpressionAtan2& to_atan2(const Expression& e);
  friend const ExpressionSinh& to_sinh(const Expression& e);
  friend const ExpressionCosh& to_cosh(const Expression& e);
  friend const ExpressionTanh& to_tanh(const Expression& e);
  friend const ExpressionMin& to_min(const Expression& e);
  friend const ExpressionMax& to_max(const Expression& e);
  friend const ExpressionCeiling& to_ceil(const Expression& e);
  friend const ExpressionFloor& to_floor(const Expression& e);
  friend const ExpressionIfThenElse& to_if_then_else(const Expression& e);
  friend const ExpressionUninterpretedFunction& to_uninterpreted_function(const Expression& e);

  friend ExpressionVar& to_variable(Expression* e);
  friend UnaryExpressionCell& to_unary(Expression* e);
  friend BinaryExpressionCell& to_binary(Expression* e);
  friend ExpressionAdd& to_addition(Expression* e);
  friend ExpressionMul& to_multiplication(Expression* e);
  friend ExpressionDiv& to_division(Expression* e);
  friend ExpressionLog& to_log(Expression* e);
  friend ExpressionAbs& to_abs(Expression* e);
  friend ExpressionExp& to_exp(Expression* e);
  friend ExpressionSqrt& to_sqrt(Expression* e);
  friend ExpressionPow& to_pow(Expression* e);
  friend ExpressionSin& to_sin(Expression* e);
  friend ExpressionCos& to_cos(Expression* e);
  friend ExpressionTan& to_tan(Expression* e);
  friend ExpressionAsin& to_asin(Expression* e);
  friend ExpressionAcos& to_acos(Expression* e);
  friend ExpressionAtan& to_atan(Expression* e);
  friend ExpressionAtan2& to_atan2(Expression* e);
  friend ExpressionSinh& to_sinh(Expression* e);
  friend ExpressionCosh& to_cosh(Expression* e);
  friend ExpressionTanh& to_tanh(Expression* e);
  friend ExpressionMin& to_min(Expression* e);
  friend ExpressionMax& to_max(Expression* e);
  friend ExpressionCeiling& to_ceil(Expression* e);
  friend ExpressionFloor& to_floor(Expression* e);
  friend ExpressionIfThenElse& to_if_then_else(Expression* e);
  friend ExpressionUninterpretedFunction& to_uninterpreted_function(Expression* e);

  friend class ExpressionAddFactory;
  friend class ExpressionMulFactory;

 private:
  explicit Expression(const std::shared_ptr<const ExpressionCell>& cell);
  void ConstructExpressionCellNaN();

  void HashAppend(DelegatingHasher* hasher) const;

  void AddImpl(const Expression& rhs);
  void SubImpl(const Expression& rhs);
  void MulImpl(const Expression& rhs);
  void DivImpl(const Expression& rhs);

  // Returns a const reference to the owned cell.
  // @pre This expression is not a Constant.
  const ExpressionCell& cell() const { return *cell_; }

  // Returns a mutable reference to the owned cell. This function may only be
  // called when this object is the sole owner of the cell (use_count == 1).
  // @pre This expression is not an ExpressionKind::Constant.
  ExpressionCell& mutable_cell();

  std::shared_ptr<const ExpressionCell> cell_;
};

Expression operator+(Expression lhs, const Expression& rhs);
// NOLINTNEXTLINE(runtime/references) per C++ standard signature.
Expression& operator+=(Expression& lhs, const Expression& rhs);
Expression operator+(const Expression& e);
Expression operator-(Expression lhs, const Expression& rhs);
// NOLINTNEXTLINE(runtime/references) per C++ standard signature.
Expression& operator-=(Expression& lhs, const Expression& rhs);
Expression operator-(const Expression& e);
Expression operator*(Expression lhs, const Expression& rhs);
// NOLINTNEXTLINE(runtime/references) per C++ standard signature.
Expression& operator*=(Expression& lhs, const Expression& rhs);
Expression operator/(Expression lhs, const Expression& rhs);
// NOLINTNEXTLINE(runtime/references) per C++ standard signature.
Expression& operator/=(Expression& lhs, const Expression& rhs);

Expression log(const Expression& e);
Expression abs(const Expression& e);
Expression exp(const Expression& e);
Expression sqrt(const Expression& e);
Expression pow(const Expression& e1, const Expression& e2);
Expression sin(const Expression& e);
Expression cos(const Expression& e);
Expression tan(const Expression& e);
Expression asin(const Expression& e);
Expression acos(const Expression& e);
Expression atan(const Expression& e);
Expression atan2(const Expression& e1, const Expression& e2);
Expression sinh(const Expression& e);
Expression cosh(const Expression& e);
Expression tanh(const Expression& e);
Expression min(const Expression& e1, const Expression& e2);
Expression max(const Expression& e1, const Expression& e2);
Expression clamp(const Expression& v, const Expression& lo, const Expression& hi);
Expression ceil(const Expression& e);
Expression floor(const Expression& e);
Expression if_then_else(const Formula& f_cond, const Expression& e_then, const Expression& e_else);

/** Constructs an uninterpreted-function expression with @p name and @p
 * arguments. An uninterpreted function is an opaque function that has no other
 * property than its name and a list of its arguments. This is useful to
 * applications where it is good enough to provide abstract information of a
 * function without exposing full details. Declaring sparsity of a system is a
 * typical example.
 */
Expression uninterpreted_function(std::string name, std::vector<Expression> arguments);
void swap(Expression& a, Expression& b);

std::ostream& operator<<(std::ostream& os, const Expression& e);

template <ExpressionKind Kind>
inline bool is_kind(const Expression& e);

/** Checks if @p e is a constant expression. */
inline bool is_constant(const Expression& e) { return is_kind<ExpressionKind::Constant>(e); }
/** Checks if @p e is a constant expression representing @p v. */
inline bool is_constant(const Expression& e, mpq_class v);
/** Checks if @p e is 0.0. */
inline bool is_zero(const Expression& e) { return is_constant(e, 0.0); }
/** Checks if @p e is 1.0. */
inline bool is_one(const Expression& e) { return is_constant(e, 1.0); }
/** Checks if @p e is -1.0. */
inline bool is_neg_one(const Expression& e) { return is_constant(e, -1.0); }
/** Checks if @p e is 2.0. */
inline bool is_two(const Expression& e) { return is_constant(e, 2.0); }

/** Returns the constant value of the constant expression @p e.
 *  \pre{@p e is a constant expression.}
 */
mpq_class get_constant_value(const Expression& e);
/** Returns the embedded variable in the variable expression @p e.
 *  \pre{@p e is a variable expression.}
 */
const Variable& get_variable(const Expression& e);
/** Returns the argument in the unary expression @p e.
 *  \pre{@p e is a unary expression.}
 */
const Expression& get_argument(const Expression& e);
/** Returns the first argument of the binary expression @p e.
 *  \pre{@p e is a binary expression.}
 */
const Expression& get_first_argument(const Expression& e);
/** Returns the second argument of the binary expression @p e.
 *  \pre{@p e is a binary expression.}
 */
const Expression& get_second_argument(const Expression& e);
/** Returns the constant part of the addition expression @p e. For instance,
 *  given 7 + 2 * x + 3 * y, it returns 7.
 *  \pre{@p e is an addition expression.}
 */
mpq_class get_constant_in_addition(const Expression& e);
/** Returns the map from an expression to its coefficient in the addition
 *  expression @p e. For instance, given 7 + 2 * x + 3 * y, the return value
 *  maps 'x' to 2 and 'y' to 3.
 *  \pre{@p e is an addition expression.}
 */
const std::map<Expression, mpq_class>& get_expr_to_coeff_map_in_addition(const Expression& e);
/** Returns the constant part of the multiplication expression @p e. For
 *  instance, given 7 * x^2 * y^3, it returns 7.
 *  \pre{@p e is a multiplication expression.}
 */
mpq_class get_constant_in_multiplication(const Expression& e);
/** Returns the map from a base expression to its exponent expression in the
 * multiplication expression @p e. For instance, given 7 * x^2 * y^3 * z^x, the
 * return value maps 'x' to 2, 'y' to 3, and 'z' to 'x'.
 *  \pre{@p e is a multiplication expression.}
 */
const std::map<Expression, Expression>& get_base_to_exponent_map_in_multiplication(const Expression& e);

/** Returns the name of an uninterpreted-function expression @p e.
 *  \pre @p e is an uninterpreted-function expression.
 */
const std::string& get_uninterpreted_function_name(const Expression& e);

/** Returns the arguments of an uninterpreted-function expression @p e.
 *  \pre @p e is an uninterpreted-function expression.
 */
const std::vector<Expression>& get_uninterpreted_function_arguments(const Expression& e);

/** Returns the conditional formula in the if-then-else expression @p e.
 * @pre @p e is an if-then-else expression.
 */
const Formula& get_conditional_formula(const Expression& e);

/** Returns the 'then' expression in the if-then-else expression @p e.
 * @pre @p e is an if-then-else expression.
 */
const Expression& get_then_expression(const Expression& e);

/** Returns the 'else' expression in the if-then-else expression @p e.
 * @pre @p e is an if-then-else expression.
 */
const Expression& get_else_expression(const Expression& e);

Expression operator+(const Variable& var);
Expression operator-(const Variable& var);

/// Returns the Taylor series expansion of `f` around `a` of order `order`.
///
/// @param[in] f     Symbolic expression to approximate using Taylor series
///                  expansion.
/// @param[in] a     Symbolic environment which specifies the point of
///                  approximation. If a partial environment is provided,
///                  the unspecified variables are treated as symbolic
///                  variables (e.g. decision variable).
/// @param[in] order Positive integer which specifies the maximum order of the
///                  resulting polynomial approximating `f` around `a`.
Expression TaylorExpand(const Expression& f, const Environment& a, int order);

}  // namespace smats

namespace std {
/* Provides std::hash<smats::Expression>. */
template <>
struct hash<smats::Expression> : public smats::DefaultHash {};

/* Provides std::less<smats::Expression>. */
template <>
struct less<smats::Expression> {
  bool operator()(const smats::Expression& lhs, const smats::Expression& rhs) const {
    return lhs.less(rhs);
  }
};

/* Provides std::equal_to<smats::Expression>. */
template <>
struct equal_to<smats::Expression> {
  bool operator()(const smats::Expression& lhs, const smats::Expression& rhs) const {
    return lhs.equal_to(rhs);
  }
};

}  // namespace std

namespace smats {

/**
 * Provides specialization of @c cond function defined in smats/common/cond.h
 * file. This specialization is required to handle @c mpq_class to @c
 * symbolic::Expression conversion so that we can write one such as <tt>cond(x >
 * 0.0, 1.0, -1.0)</tt>.
 */
template <typename... Rest>
symbolic::Expression cond(const symbolic::Formula& f_cond, mpq_class v_then, Rest... rest) {
  return if_then_else(f_cond, symbolic::Expression{v_then}, cond(rest...));
}

/// Returns the symbolic expression's value() as a mpq_class.
///
/// @throws std::exception if it is not possible to evaluate the symbolic
/// expression with an empty environment.
mpq_class ExtractDoubleOrThrow(const symbolic::Expression& e);

}  // namespace smats