/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/expression.h"

#include <ostream>
#include <sstream>

#include "smats/symbolic/expression_cell.h"
#include "smats/symbolic/expression_factory.h"
#include "smats/util/error.h"

namespace smats {

template <class T>
Expression<T>::Expression() : cell_{Expression<T>::zero().cell().ptr()} {}

template <class T>
Expression<T>::Expression(const T& value)
    : cell_{value == 0   ? Expression<T>::zero().cell().ptr()
            : value == 1 ? Expression<T>::one().cell().ptr()
                         : ExpressionConstant<T>::New(value)} {}

template <class T>
Expression<T>::Expression(const Variable& var) : cell_{ExpressionVar<T>::New(var)} {}

template <class T>
Expression<T>::Expression(const std::shared_ptr<const ExpressionCell<T>>& cell) : cell_{cell} {}

template <class T>
ExpressionKind Expression<T>::kind() const {
  return cell_->kind();
}
template <class T>
Variables Expression<T>::variables() const {
  return cell_->variables();
}

template <class T>
bool Expression<T>::equal_to(const Expression<T>& e) const {
  return false;
}
template <class T>
bool Expression<T>::less(const Expression<T>& e) const {
  return cell_->less(*e.cell_);
}
template <class T>
bool Expression<T>::is_polynomial() const {
  return cell_->is_polynomial();
}

template <class T>
bool Expression<T>::is_constant() const {
  return cell_->kind() == ExpressionKind::Constant;
}
template <class T>
bool Expression<T>::is_constant(const T& value) const {
  return cell_->kind() == ExpressionKind::Constant && cell_->template to<ExpressionConstant>().value() == value;
}
template <class T>
bool Expression<T>::is_variable() const {
  return cell_->kind() == ExpressionKind::Var;
}
template <class T>
bool Expression<T>::is_variable(const Variable& var) const {
  return cell_->kind() == ExpressionKind::Var && cell_->template to<ExpressionVar>().variable().equal_to(var);
}
template <class T>
bool Expression<T>::is_addition() const {
  return cell_->kind() == ExpressionKind::Add;
}
template <class T>
bool Expression<T>::is_multiplication() const {
  return cell_->kind() == ExpressionKind::Mul;
}
template <class T>
bool Expression<T>::is_division() const {
  return cell_->kind() == ExpressionKind::Div;
}
template <class T>
bool Expression<T>::is_nan() const {
  return cell_->kind() == ExpressionKind::NaN;
}
template <class T>
bool Expression<T>::is_pow() const {
  return cell_->kind() == ExpressionKind::Pow;
}

template <class T>
T Expression<T>::evaluate(const Environment<T>& env) const {
  return cell_->evaluate(env);
}
template <class T>
Expression<T> Expression<T>::evaluate_partial(const Environment<T>& env) const {
  return cell_->evaluate_partial(env);
}
template <class T>
bool Expression<T>::is_expanded() const {
  return cell_->is_expanded();
}
template <class T>
Expression<T> Expression<T>::expand() const {
  return cell_->expand();
}
template <class T>
Expression<T> Expression<T>::substitute(const Variable& var, const Expression<T>& e) const {
  return cell_->substitute(Substitution<T>{{var, e}});
}
template <class T>
Expression<T> Expression<T>::substitute(const Substitution<T>& s) const {
  return cell_->substitute(s);
}
template <class T>
Expression<T> Expression<T>::differentiate(const Variable& x) const {
  return cell_->differentiate(x);
}
template <class T>
std::string Expression<T>::to_string() const {
  std::stringstream str{};
  cell_->display(str);
  return str.str();
}

template <class T>
const T& Expression<T>::constant() const {
  SMATS_ASSERT(is_constant() || is_addition() || is_multiplication(), "Expression does not have a constant");
  switch (cell_->kind()) {
    case ExpressionKind::Constant:
      return cell_->template to<ExpressionConstant>().value();
    case ExpressionKind::Add:
      return cell_->template to<ExpressionAdd>().constant();
    case ExpressionKind::Mul:
      return cell_->template to<ExpressionMul>().constant();
    default:
      SMATS_UNREACHABLE();
  }
}

template <class T>
const std::map<Expression<T>, T>& Expression<T>::expression_to_coeff_map() const {
  SMATS_ASSERT(is_addition(), "Expression is not an addition");
  return cell_->template to<ExpressionAdd>().expr_to_coeff_map();
}

template <class T>
Expression<T> Expression<T>::zero() {
  static const Expression<T> zero{ExpressionConstant<T>::New(0)};
  return zero;
}
template <class T>
Expression<T> Expression<T>::one() {
  static const Expression<T> one{ExpressionConstant<T>::New(1)};
  return one;
}
template <class T>
Expression<T> Expression<T>::NaN() {
  static const Expression<T> nan{ExpressionNaN<T>::New()};
  return nan;
}

template <class T>
ExpressionCell<T>& Expression<T>::m_cell() {
  SMATS_ASSERT(cell_.use_count() == 1, "Expression is not the only owner of the cell");
  return const_cast<ExpressionCell<T>&>(*cell_);
}

template <class T>
Expression<T> Expression<T>::operator-() const {
  // Simplification: constant folding
  if (is_constant()) return Expression{-constant()};
  // Simplification: push '-' inside over '+'.
  // -(E_1 + ... + E_n) => (-E_1 + ... + -E_n)
  if (is_addition()) {
    ExpressionAddFactory<T> factory{*this};
    factory.negate();
    return factory.build();
  }
  // Simplification: push '-' inside over '*'.
  // -(c0 * E_1 * ... * E_n) => (-c0 * E_1 * ... * E_n)
  if (is_multiplication()) {
    ExpressionMulFactory<T> factory{*this};
    factory.negate();
    return factory.build();
  }
  return Expression<T>{-1} * *this;
}

template <class T>
Expression<T>& Expression<T>::operator+=(const Expression& o) {
  // Simplification: 0 + E => E
  if (is_constant(0)) return *this = o;
  // Simplification: E + 0 => E
  if (o.is_constant(0)) return *this;
  // Simplification: E(c1) + E(c2) => E(c1 + c2)
  if (is_constant() && o.is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() += o.constant();
      return *this;
    } else {
      return *this = Expression{constant() + o.constant()};
    }
  }
  ExpressionAddFactory<T> factory{is_addition() ? *this : o};
  factory += is_addition() ? o : *this;
  return *this = factory.build();
}
template <class T>
Expression<T>& Expression<T>::operator-=(const Expression<T>& o) {
  if (is_constant() && o.is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() -= o.constant();
      return *this;
    } else {
      return *this = Expression{constant() - o.constant()};
    }
  }
  return *this += -o;
}
template <class T>
Expression<T>& Expression<T>::operator*=(const Expression<T>& o) {
  // Simplification: 1 * E => x
  if (is_constant(1)) return *this = o;
  // Simplification: E * 1 => x
  if (o.is_constant(1)) return *this;
  // Simplification: 0 * E => 0
  if (is_constant(0)) return *this;
  // Simplification: E * 0 => 0
  if (o.is_constant(0)) return *this = Expression::zero();
  // Simplification: E(c1) * E(c2) => E(c1 * c2)
  if (is_constant() && o.is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() *= o.constant();
      return *this;
    } else {
      return *this = Expression{constant() * o.constant()};
    }
  }
  // Simplification: E * -1 => -E
  if (is_constant(-1)) {
    if (o.is_addition()) {
      ExpressionAddFactory<T> factory{o};
      factory.negate();
      return *this = factory.build();
    }
    if (o.is_multiplication()) {
      ExpressionMulFactory<T> factory{o};
      factory.negate();
      return *this = factory.build();
    }
  }
  // Simplification: -1 * E => -E
  if (o.is_constant(-1)) {
    if (is_addition()) {
      ExpressionAddFactory<T> factory{*this};
      factory.negate();
      return *this = factory.build();
    }
    if (is_multiplication()) {
      ExpressionMulFactory<T> factory{*this};
      factory.negate();
      return *this = factory.build();
    }
  }
  SMATS_UNREACHABLE();
}
template <class T>
Expression<T>& Expression<T>::operator/=(const Expression<T>& o) {
  // Simplification: E / 1 => x
  if (o.is_constant(1)) return *this;
  // Simplification: 0 / E => 0
  if (is_constant(0)) return *this;
  // Simplification: E / 0 => NaN
  if (o.is_constant(0)) return *this = Expression::NaN();
  // Simplification: E(c1) / E(c2) => E(c1 / c2)
  if (is_constant() && o.is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() /= o.constant();
      return *this;
    } else {
      return *this = Expression{constant() / o.constant()};
    }
  }
  SMATS_UNREACHABLE();
}

template <class T>
Expression<T>& Expression<T>::operator++() {
  *this += Expression::one();
  return *this;
}
template <class T>
Expression<T> Expression<T>::operator++(int) {
  Expression tmp{*this};
  *this += Expression::one();
  return tmp;
}
template <class T>
Expression<T>& Expression<T>::operator--() {
  *this -= Expression::one();
  return *this;
}
template <class T>
Expression<T> Expression<T>::operator--(int) {
  Expression tmp{*this};
  *this -= Expression::one();
  return tmp;
}
template <class T>
Expression<T>& Expression<T>::operator^=(const Expression<T>& o) {
  // Simplification
  if (o.is_constant()) {
    const T& exp_value = o.constant();
    if (is_constant()) {
      // Constant folding
      const T& base_value = constant();
      ExpressionPow<T>::check_domain(base_value, exp_value);
      return *this = Expression<T>{static_cast<T>(std::pow(base_value, exp_value))};
    }
    // pow(E, 0) => 1
    // TODO(soonho-tri): This simplification is not sound since it cancels `E` which might contain 0/0 problems.
    if (exp_value == 0) return *this = Expression::one();
    // pow(E, 1) => E
    if (exp_value == 1) return *this;
  }
  if (is_pow() && o.is_constant()) {
    // pow(base, exponent) ^ e2 => pow(base, exponent * e2)
    // only if both of exponent and e2 are integers.
    const Expression<T>& pow_exponent{o.lhs()};
    const T& pow_exponent_value = pow_exponent.constant();
    const T& exponent_value = o.constant();
    if (is_integer(v1) && is_integer(v2)) {
      const Expression& base{get_first_argument(e1)};
      return Expression{new ExpressionPow(base, v1 * v2)};
    }
  }
  return Expression<T>{ExpressionPow::New(e1, e2)};
}
template <class T>
Expression<T> Expression<T>::operator^(const Expression<T>& o) const {
  Expression tmp{*this};
  tmp ^= o;
  return tmp;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Expression<T>& expr) {
  return os << expr.to_string();
}

EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(Expression);

template std::ostream& operator<<(std::ostream& os, const Expression<int>& expr);
template std::ostream& operator<<(std::ostream& os, const Expression<long>& expr);
template std::ostream& operator<<(std::ostream& os, const Expression<double>& expr);
template std::ostream& operator<<(std::ostream& os, const Expression<float>& expr);

}  // namespace smats
