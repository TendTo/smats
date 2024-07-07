#pragma once

#include <algorithm>  // for cpplint only
#include <atomic>
#include <cstddef>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "smats/symbolic/environment.h"
#include "smats/symbolic/expression.h"
#include "smats/symbolic/expression_kind.h"
#include "smats/symbolic/variables.h"
#include "smats/util/definitions.h"
#include "smats/util/hash.hpp"

namespace smats {

// Forward declaration.
template <class T>
class Expression;

template <class T>
using Substitution = std::unordered_map<Variable, Expression<T>>;

#define PARAMS(...) __VA_ARGS__
#define NEW_OPERATOR(class_name)                                                \
  static std::shared_ptr<const class_name<T>> New() {                           \
    return std::make_shared<const class_name<T>>(ExpressionCell<T>::Private()); \
  }                                                                             \
  explicit class_name(ExpressionCell<T>::Private)

#define NEW_OPERATOR_PARAMS(class_name, params, args)                                 \
  static std::shared_ptr<const class_name<T>> New(params) {                           \
    return std::make_shared<const class_name<T>>(ExpressionCell<T>::Private(), args); \
  }                                                                                   \
  class_name(ExpressionCell<T>::Private, params)

/**
 * Represents an abstract class which is the base of concrete symbolic-expression classes.
 *
 * @note It provides virtual function, ExpressionCell::Display,
 * because operator<< is not allowed to be a virtual function.
 * @note All subclasses of ExpressionCell must be immutable and implement the static function
 * @code
 * static std::shared_ptr<ExpressionCell> create() { return std::make_shared<ExpressionCell>(Private()); }
 * @endcode
 * to create instances of the ExpressionCell externally while guaranteeing the use of shared_ptr.
 */
template <class T>
class ExpressionCell : public std::enable_shared_from_this<ExpressionCell<T>> {
 public:
  ExpressionCell() = delete;
  virtual ~ExpressionCell() = default;

  std::shared_ptr<ExpressionCell<T>> ptr() { return shared_from_this<ExpressionCell<T>>(); }

  /** @getter{kind, expression cell} */
  [[nodiscard]] ExpressionKind kind() const { return kind_; }

  /**
   * Sends all hash-relevant bytes for this ExpressionCell type into the given
   * hasher, per the @ref hash_append concept -- except for kind(), because
   * Expression already sends that.
   */
  virtual void hash(DelegatingHasher&) const = 0;

  /** @getter{variables, expression cell} */
  [[nodiscard]] virtual Variables variables() const = 0;

  /** @equal_to{expression cells} */
  [[nodiscard]] virtual bool equal_to(const ExpressionCell<T>& c) const = 0;

  /** @less{expression cells} */
  [[nodiscard]] virtual bool less(const ExpressionCell<T>& c) const = 0;

  /** @checker{polynomial, expression cell} */
  [[nodiscard]] bool is_polynomial() const { return is_polynomial_; }

  /** @checker{expanded, expression cell} */
  [[nodiscard]] bool is_expanded() const { return is_expanded_; }

  /** Sets this symbolic expression as already expanded. */
  void set_expanded() { is_expanded_ = true; }

  template <template <class> class E>
  [[nodiscard]] const E<T>& to() {
#ifndef NDEBUG
    if (kind() != E<T>::class_kind) throw std::runtime_error("ExpressionCell::to: invalid cast");
#endif
    return static_cast<const E<T>&>(*this);
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

  /** Outputs string representation of expression into output stream @p os. */
  virtual std::ostream& display(std::ostream& os) const = 0;

 protected:
  struct Private {};
  /**
   * Constructs ExpressionCell of kind @p kind with @p is_polynomial and @p is_expanded .
   * @param kind kind of the expression
   * @param is_polynomial whether the expression is a polynomial
   * @param is_expanded whether the expression is already expanded
   */
  ExpressionCell(ExpressionKind kind, bool is_polynomial, bool is_expanded);

 private:
  const ExpressionKind kind_;  ///< The kind of the expression.
  const bool is_polynomial_;   ///< Whether the expression is a polynomial.
  bool is_expanded_{false};    ///< Whether the expression is already expanded.
};

/** Represents the base class for unary expressions.  */
template <class T>
class UnaryExpressionCell : public ExpressionCell<T> {
 public:
  void hash(DelegatingHasher&) const override;
  [[nodiscard]] Variables variables() const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& e) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& e) const override;
  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  /** Returns the argument. */
  [[nodiscard]] const Expression<T>& get_argument() const { return e_; }

 protected:
  /** Constructs UnaryExpressionCell of kind @p k with @p e, @p is_poly, and @p is_expanded. */
  UnaryExpressionCell(ExpressionKind k, Expression<T> e, bool is_poly, bool is_expanded);
  /** Returns the evaluation result f(@p v ). */
  [[nodiscard]] virtual T do_evaluate(T v) const = 0;

 private:
  const Expression<T> e_;
};

/** Represents the base class for binary expressions.
 */
template <class T>
class BinaryExpressionCell : public ExpressionCell<T> {
 public:
  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] Variables variables() const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& e) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& e) const override;
  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  /** Returns the first argument. */
  [[nodiscard]] const Expression<T>& get_first_argument() const { return e1_; }
  /** Returns the second argument. */
  [[nodiscard]] const Expression<T>& get_second_argument() const { return e2_; }

 protected:
  /** Constructs BinaryExpressionCell of kind @p k with @p e1, @p e2,
   * @p is_poly, and @p is_expanded.
   */
  BinaryExpressionCell(ExpressionKind k, Expression<T> e1, Expression<T> e2, bool is_poly, bool is_expanded);
  /** Returns the evaluation result f(@p v1, @p v2 ). */
  [[nodiscard]] virtual T do_evaluate(T v1, T v2) const = 0;

 private:
  const Expression<T> e1_;
  const Expression<T> e2_;
};

/**
 * Symbolic expression representing a constant.
 * @tparam T type of the constant
 */
template <class T>
class ExpressionConstant : public ExpressionCell<T> {
 public:
  static const ExpressionKind class_kind = ExpressionKind::Constant;

  NEW_OPERATOR_PARAMS(ExpressionConstant, const T& constant, constant);

  [[nodiscard]] const T& value() const { return value_; }
  [[nodiscard]] Variables variables() const override;

  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& e) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& e) const override;

  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;

 private:
  const T value_{};
};

/** Symbolic expression representing a variable. */
template <class T>
class ExpressionVar : public ExpressionCell<T> {
 public:
  static const ExpressionKind class_kind = ExpressionKind::Var;

  NEW_OPERATOR_PARAMS(ExpressionVar, Variable v, v);

  [[nodiscard]] const Variable& variable() const { return var_; }
  [[nodiscard]] Variables variables() const override;

  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& e) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& e) const override;

  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;

 private:
  const Variable var_;  ///< Variable contained in the Expression. Cannot be a Boolean variable
};

/** Symbolic expression representing NaN (not-a-number). */
template <class T>
class ExpressionNaN : public ExpressionCell<T> {
 public:
  static const ExpressionKind class_kind = ExpressionKind::NaN;

  NEW_OPERATOR(ExpressionNaN);

  void hash(DelegatingHasher& hasher) const override;
  [[nodiscard]] Variables variables() const override;
  [[nodiscard]] bool equal_to(const ExpressionCell<T>& e) const override;
  [[nodiscard]] bool less(const ExpressionCell<T>& e) const override;
  [[nodiscard]] T evaluate(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> expand() const override;
  [[nodiscard]] Expression<T> evaluate_partial(const Environment<T>& env) const override;
  [[nodiscard]] Expression<T> substitute(const Substitution<T>& s) const override;
  [[nodiscard]] Expression<T> differentiate(const Variable& x) const override;
  std::ostream& display(std::ostream& os) const override;
};

EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionCell);
EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionConstant);
EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionVar);
EXTERNAL_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionNaN);

}  // namespace smats
