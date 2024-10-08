/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * ExpressionCell class and its subclasses.
 */
#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "smats/symbolic/environment.h"
#include "smats/symbolic/expression.h"
#include "smats/symbolic/expression_kind.h"
#include "smats/symbolic/variable.h"
#include "smats/symbolic/variables.h"
#include "smats/util/definitions.h"
#include "smats/util/hash.hpp"

namespace smats {

template <class T>
using Substitution = std::unordered_map<Variable, Expression<T>>;

#define PARAMS(...) __VA_ARGS__
#define NEW_OPERATOR(class_name)                                                         \
  static std::shared_ptr<const class_name<T>> New() {                                    \
    return std::make_shared<const class_name<T>>(typename ExpressionCell<T>::Private()); \
  }                                                                                      \
  explicit class_name(typename ExpressionCell<T>::Private)

#define NEW_OPERATOR_PARAMS(class_name, params, args)                                          \
  static std::shared_ptr<const class_name<T>> New(params) {                                    \
    return std::make_shared<const class_name<T>>(typename ExpressionCell<T>::Private(), args); \
  }                                                                                            \
  class_name(typename ExpressionCell<T>::Private, params)

/**
 * Abstract class used as the base of concrete symbolic-expression classes.
 *
 * @note It provides virtual function, ExpressionCell<T>::display because operator<< is not allowed to be virtual.
 * @note All subclasses of ExpressionCell must be immutable and implement the static function
 * @code
 * static std::shared_ptr<ExpressionCell> create() { return std::make_shared<ExpressionCell>(Private()); }
 * @endcode
 * to create instances of the ExpressionCell externally while guaranteeing the use of shared_ptr.
 * @tparam T type of the expression evaluation
 */
template <class T>
class ExpressionCell : public std::enable_shared_from_this<ExpressionCell<T>> {
 public:
  ExpressionCell() = delete;
  virtual ~ExpressionCell() = default;

  /** @getter{reference count, expression cell} */
  [[nodiscard]] long use_count() const {
    return std::enable_shared_from_this<ExpressionCell<T>>::shared_from_this().use_count();
  }

  /** @getter{kind, expression cell} */
  [[nodiscard]] ExpressionKind kind() const { return kind_; }

  /**
   * Sends all hash-relevant bytes for this ExpressionCell type into the given hasher,
   * per the @ref hash_append concept, except for kind(), because Expression already sends that.
   */
  virtual void hash(DelegatingHasher&) const = 0;

  /** @getter{variables, expression cell} */
  [[nodiscard]] const Variables& variables() const;

  /** @checker{polynomial, expression cell} */
  [[nodiscard]] bool is_polynomial() const;

  /** @checker{expanded, expression cell} */
  [[nodiscard]] bool is_expanded() const { return is_expanded_; }

  /** @equal_to{expression cells} */
  [[nodiscard]] virtual bool equal_to(const ExpressionCell<T>& o) const = 0;

  /** @less{expression cells} */
  [[nodiscard]] virtual bool less(const ExpressionCell<T>& o) const = 0;

  /** Sets this symbolic expression as already expanded. */
  void set_expanded() { is_expanded_ = true; }

  /**
   * Casts this ExpressionCell to a subclass of ExpressionCell with the same template type.
   * @tparam E subclass of ExpressionCell
   * @return constant reference to the casted ExpressionCell
   */
  template <template <class> class E>
  [[nodiscard]] const E<T>& to() const {
#ifndef NDEBUG
    if (kind() != E<T>::expression_kind) throw std::runtime_error("ExpressionCell::to(): invalid cast");
#endif
    return static_cast<const E<T>&>(*this);
  }
  /**
   * Casts this ExpressionCell to a subclass of ExpressionCell with the same template type.
   * @tparam E subclass of ExpressionCell
   * @return mutable reference to the casted ExpressionCell
   */
  template <template <class> class E>
  [[nodiscard]] E<T>& to() {
#ifndef NDEBUG
    if (kind() != E<T>::expression_kind) throw std::runtime_error("ExpressionCell::to(): invalid cast");
#endif
    return static_cast<E<T>&>(*this);
  }

  /**
   * Create an Expression from this ExpressionCell.
   * @return Expression containing this ExpressionCell
   */
  [[nodiscard]] Expression<T> to_expression() const {
    return Expression{std::enable_shared_from_this<ExpressionCell<T>>::shared_from_this()};
  }

  /**
   * Evaluates under a given environment (by default, an empty environment).
   * @param env environment
   * @throws std::runtime_exception if NaN is detected during evaluation.
   */
  [[nodiscard]] virtual T evaluate(const Environment<T>& env) const = 0;

  /**
   * Expands out products and positive integer powers in expression.
   * @throws std::runtime_exception if NaN is detected during expansion.
   */
  [[nodiscard]] virtual Expression<T> expand() const = 0;

  /**
   * Returns an Expression obtained by replacing all occurrences of the
   * variables in @p env in the current expression cell with the corresponding
   * values in @p env.
   * @param env environment
   * @throws std::runtime_exception if NaN is detected during substitution.
   */
  [[nodiscard]] virtual Expression<T> evaluate_partial(const Environment<T>& env) const = 0;

  /**
   * Returns an Expression obtained by replacing all occurrences of the
   * variables in @p s in the current expression cell with the corresponding
   * expressions in @p s.
   * @param s substitution
   * @throws std::runtime_exception if NaN is detected during substitution.
   */
  [[nodiscard]] virtual Expression<T> substitute(const Substitution<T>& s) const = 0;

  /**
   * Differentiates this symbolic expression with respect to the variable @p var.
   * @param x variable
   * @throws std::runtime_exception if it is not differentiable.
   */
  [[nodiscard]] virtual Expression<T> differentiate(const Variable& x) const = 0;

  /**
   * Displays the expression in a human-readable format.
   * @param os output stream
   * @return updated output stream
   */
  virtual std::ostream& display(std::ostream& os) const = 0;

  /**
   * Invalidate the cached information.
   *
   * This will force the expression to recompute the cached information the next time they are requested.
   * Must be called whenever the expression is modified.
   */
  void invalidate_cache();

 protected:
  /** Struct used to ensure that only subclasses can create instances of ExpressionCell. */
  struct Private {};
  /**
   * Constructs ExpressionCell of @p kind with @p is_expanded .
   * @param kind kind of the expression
   * @param is_expanded whether the expression is already expanded
   */
  ExpressionCell(ExpressionKind kind, bool is_expanded);
  /**
   * Constructs ExpressionCell of @p kind with @p is_polynomial and @p is_expanded .
   * @param kind kind of the expression
   * @param is_polynomial whether the expression is a polynomial
   * @param is_expanded whether the expression is already expanded
   */
  ExpressionCell(ExpressionKind kind, bool is_polynomial, bool is_expanded);

  virtual void compute_variables(std::optional<Variables>& variables) const = 0;
  virtual void compute_is_polynomial(std::optional<bool>& is_polynomial) const = 0;

 private:
  const ExpressionKind kind_;                   ///< The kind of the expression.
  mutable std::optional<Variables> variables_;  ///< Cached variables in the expression.
  mutable std::optional<bool> is_polynomial_;   ///< Cached information about whether the expression is a polynomial.
  bool is_expanded_{false};                     ///< Whether the expression is already expanded.
};

/**
 *Symbolic expression used as a base for unary expressions.
 *
 * The expression holds another expression internally.
 * Some example include
 * @f[
 * \log(x) \\
 * \exp(x) \\
 * \sin(x) \\
 * \cos(x) \\
 * \tan(x) \\
 * @f]
 * @tparam T type of the expression evaluation
 */
template <class T>
class UnaryExpressionCell : public ExpressionCell<T> {
 public:
  /** @getter{argument, unary expression} */
  [[nodiscard]] const Expression<T>& get_argument() const { return e_; }

 protected:
  /**
   * Construct a new UnaryExpressionCell of @p kind with @p e, @p is_polynomial, and @p is_expanded.
   * @param kind kind of the expression
   * @param e expression
   * @param is_expanded whether the expression is already expanded
   */
  UnaryExpressionCell(ExpressionKind k, Expression<T> e, bool is_expanded);
  /**
   * Construct a new UnaryExpressionCell of @p kind with @p e, @p is_polynomial, and @p is_expanded.
   * @param kind kind of the expression
   * @param e expression
   * @param is_polynomial whether the expression is a polynomial
   * @param is_expanded whether the expression is already expanded
   */
  UnaryExpressionCell(ExpressionKind k, Expression<T> e, bool is_polynomial, bool is_expanded);
  /**
   * Evaluate the unary expression with the given value @p v of the argument.
   * @param v value
   * @return evaluation result
   */
  [[nodiscard]] virtual T do_evaluate(T v) const = 0;

 private:
  const Expression<T> e_;  ///< The argument of the unary expression.
};

/**
 * Symbolic expression used as a base for binary expressions.
 *
 * The expression holds two other expressions internally.
 * Some example include
 * @f[
 * \min(x, y) \\
 * \max(x, y) \\
 * x^y \\
 * x \ y \\
 * @f]
 * @tparam T type of the expression evaluation
 */
template <class T>
class BinaryExpressionCell : public ExpressionCell<T> {
 public:
  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& o) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& o) const override;
  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  /** @getter{first argument, binary expression} */
  [[nodiscard]] const Expression<T>& lhs() const { return e1_; }
  /** @getter{second argument, binary expression} */
  [[nodiscard]] const Expression<T>& rhs() const { return e2_; }

 protected:
  /**
   * Constructs BinaryExpressionCell of @p kind with @p e1, @p e2 @p is_polynomial, and @p is_expanded.
   * @param kind kind of the expression
   * @param e1 first expression
   * @param e2 second expression
   * @param is_expanded whether the expression is already expanded
   */
  BinaryExpressionCell(ExpressionKind kind, Expression<T> e1, Expression<T> e2, bool is_expanded);
  /**
   * Evaluate the binary expression, given the two values @p v1 and @p v2 of the arguments.
   * @param v1 first value
   * @param v2 second value
   * @return evaluation result
   */
  [[nodiscard]] virtual T do_evaluate(const T& v1, const T& v2) const = 0;

  void compute_variables(std::optional<Variables>& variables) const override;
  void compute_is_polynomial(std::optional<bool>& is_polynomial) const override;

 private:
  const Expression<T> e1_;  ///< The first argument of the binary expression.
  const Expression<T> e2_;  ///< The second argument of the binary expression.
};

/**
 * Symbolic expression representing a constant value.
 *
 * The expression holds a constant value of type @p T internally.
 * @tparam T type of the constant value
 */
template <class T>
class ExpressionConstant : public ExpressionCell<T> {
 public:
  static const ExpressionKind expression_kind = ExpressionKind::Constant;

  NEW_OPERATOR_PARAMS(ExpressionConstant, const T& constant, constant);

  /** @getter{value, constant expression} */
  [[nodiscard]] const T& value() const { return value_; }
  /** @getsetter{value, constant expression} */
  T& m_value();

  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& o) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& o) const override;

  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;

 private:
  const T value_;  ///< Constant value

  void compute_variables(std::optional<Variables>& variables) const override;
  void compute_is_polynomial(std::optional<bool>& is_polynomial) const override;
};

/**
 * Symbolic expression representing a variable.
 *
 * The expression holds a variable internally.
 * @tparam T type of the expression evaluation
 */
template <class T>
class ExpressionVar : public ExpressionCell<T> {
 public:
  static const ExpressionKind expression_kind = ExpressionKind::Var;

  NEW_OPERATOR_PARAMS(ExpressionVar, Variable v, v);

  /** @getter{variable, variable expression} */
  [[nodiscard]] const Variable& variable() const { return var_; }

  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& o) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& o) const override;

  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;

 private:
  const Variable var_;  ///< Variable contained in the Expression. Cannot be a Boolean variable

  void compute_variables(std::optional<Variables>& variables) const override;
  void compute_is_polynomial(std::optional<bool>& is_polynomial) const override;
};

/**
 * Symbolic expression representing NaN (not-a-number)
 *
 * Any attempt at evaluating this expression will result in an exception.
 * @tparam T type of the expression evaluation
 */
template <class T>
class ExpressionNaN : public ExpressionCell<T> {
 public:
  static const ExpressionKind expression_kind = ExpressionKind::NaN;

  NEW_OPERATOR(ExpressionNaN);

  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& o) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& o) const override;
  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;

 private:
  void compute_variables(std::optional<Variables>& variables) const override;
  void compute_is_polynomial(std::optional<bool>& is_polynomial) const override;
};

/**
 * Symbolic expression representing the addition of multiple terms.
 *
 * The expression holds a constant term and a map from expressions to their coefficients.
 * Let @f$ k @f$ be the constant term and @f$ E @f$ be the set of pairs @f$(c, e)@f$ where @f$ c @f$ is the coefficient
 * and @f$ e @f$ is the expression.
 * The ExpressionAdd cell represents the following expression:
 * @f[
 * k + \sum_{(c, e) \in E} c \cdot e
 * @f]
 * @tparam T type of the expression evaluation
 */
template <class T>
class ExpressionAdd : public ExpressionCell<T> {
 public:
  using ExpressionMap = std::map<Expression<T>, T>;
  static const ExpressionKind expression_kind = ExpressionKind::Add;

  NEW_OPERATOR_PARAMS(ExpressionAdd, PARAMS(T constant, ExpressionMap expr_to_coeff_map),
                      PARAMS(constant, expr_to_coeff_map));

  void hash(DelegatingHasher&) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& o) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& o) const override;
  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;
  /** @getter{constant, addition expression} */
  [[nodiscard]] const T& constant() const { return constant_; }
  /** @getsetter{constant, addition expression} */
  T& m_constant();

  /** @getter{map, addition expression} */
  [[nodiscard]] const ExpressionMap& expr_to_coeff_map() const { return expr_to_coeff_map_; }
  /** @getsetter{map, addition expression} */
  ExpressionMap& m_expr_to_coeff_map();

 private:
  static Variables extract_variables(const ExpressionMap& expr_to_coeff_map);

  const T constant_;                       ///< Constant term of the addition.
  const ExpressionMap expr_to_coeff_map_;  ///< Map from expressions to their coefficients.

  void compute_variables(std::optional<Variables>& variables) const override;
  void compute_is_polynomial(std::optional<bool>& is_polynomial) const override;
  /**
   * Utility function used to display a single term in a pretty format.
   * @param os output stream
   * @param print_plus whether to print a plus at the beginning
   * @param coeff coefficient of the term
   * @param term expression of the term
   * @return updated output stream
   */
  std::ostream& display_term(std::ostream& os, bool print_plus, const T& coeff, const Expression<T>& term) const;
};

/**
 * Symbolic expression representing a multiplication of powers.
 *
 * The expression holds a constant term and a map from expressions to their coefficients.
 * Let @f$ k @f$ be the constant term and @f$ E @f$ be the set of pairs @f$(b, e)@f$ where @f$ b @f$ is the base
 * and @f$ e @f$ is the exponent.
 * The ExpressionMul cell represents the following expression:
 * @f[
 *     k \cdot \prod_{(b, e) \in E} b^e
 * @f]
 */
template <class T>
class ExpressionMul : public ExpressionCell<T> {
 public:
  using ExpressionMap = std::map<Expression<T>, Expression<T>>;
  static const ExpressionKind expression_kind = ExpressionKind::Mul;

  NEW_OPERATOR_PARAMS(ExpressionMul, PARAMS(T constant, ExpressionMap expr_to_coeff_map),
                      PARAMS(constant, expr_to_coeff_map));

  void hash(DelegatingHasher&) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& o) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& o) const override;
  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;
  /** @getter{constant, multiplication expression} */
  [[nodiscard]] const T& constant() const { return constant_; }
  /** @getsetter{constant, multiplication expression} */
  T& m_constant();

  /** @getter{map, multiplication expression} */
  [[nodiscard]] const ExpressionMap& base_to_exponent_map() const { return base_to_exponent_map_; }
  /** @getsetter{map, multiplication expression} */
  ExpressionMap& m_base_to_exponent_map();

 private:
  static Variables extract_variables(const ExpressionMap& base_to_exponent_map);

  const T constant_;                          ///< Constant term of the multiplication.
  const ExpressionMap base_to_exponent_map_;  ///< Map from expressions to their exponent.

  void compute_variables(std::optional<Variables>& variables) const override;
  void compute_is_polynomial(std::optional<bool>& is_polynomial) const override;

  /**
   * Utility function used to display a single term in a pretty format.
   * @param os output stream
   * @param print_mul whether to print a multiplication symbol at the beginning
   * @param base base of the term
   * @param exponent exponent of the term
   * @return updated output stream
   */
  std::ostream& display_term(std::ostream& os, bool print_mul, const Expression<T>& base,
                             const Expression<T>& exponent) const;
};

/**
 * Symbolic expression representing the power function.
 *
 * The expression holds two other expressions internally.
 * Let @f$ x @f$ be the first expression and @f$ y @f$ be the second expression.
 * The ExpressionPow cell represents the following expression:
 * @f[
 * x^y
 * @f]
 * @tparam T type of the expression evaluation
 */
template <class T>
class ExpressionPow : public BinaryExpressionCell<T> {
 public:
  static const ExpressionKind expression_kind = ExpressionKind::Pow;
  static void check_domain(const T& v1, const T& v2);

  NEW_OPERATOR_PARAMS(ExpressionPow, PARAMS(const Expression<T>& e1, const Expression<T>& e2), PARAMS(e1, e2));

  Expression<T> evaluate_partial(const Environment<T>& env) const override;
  Expression<T> expand() const override;
  Expression<T> substitute(const Substitution<T>& s) const override;
  Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;

 private:
  T do_evaluate(const T& v1, const T& v2) const override;

  void compute_is_polynomial(std::optional<bool>& is_polynomial) const override;
};

/**
 * Symbolic expression representing the power function.
 *
 * The expression holds two other expressions internally.
 * Let @f$ x @f$ be the first expression and @f$ y @f$ be the second expression.
 * The ExpressionPow cell represents the following expression:
 * @f[
 * x^y
 * @f]
 * @tparam T type of the expression evaluation
 */
template <class T>
class ExpressionDiv : public BinaryExpressionCell<T> {
 public:
  static const ExpressionKind expression_kind = ExpressionKind::Div;
  static void check_domain(const T& v1, const T& v2);

  NEW_OPERATOR_PARAMS(ExpressionDiv, PARAMS(const Expression<T>& e1, const Expression<T>& e2), PARAMS(e1, e2));

  Expression<T> expand() const override;
  Expression<T> evaluate_partial(const Environment<T>& env) const override;
  Expression<T> substitute(const Substitution<T>& s) const override;
  Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;

 private:
  T do_evaluate(const T& v1, const T& v2) const override;
};

// EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionCell);
// EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionConstant);
// EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionVar);
// EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionNaN);
// EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionAdd);
// EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionMul);
// EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionPow);

}  // namespace smats
