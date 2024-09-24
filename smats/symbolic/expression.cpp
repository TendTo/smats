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
#include "smats/symbolic/symbolic_util.h"
#include "smats/util/error.h"

namespace smats {

template <class T>
Expression<T>::Expression() : cell_{Expression<T>::zero().cell().shared_from_this()} {}

template <class T>
Expression<T>::Expression(const T& value)
    : cell_{value == 0   ? Expression<T>::zero().cell().shared_from_this()
            : value == 1 ? Expression<T>::one().cell().shared_from_this()
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
  if (cell_ == e.cell_) return true;
  if (kind() != e.kind()) return false;
  return cell_->equal_to(*e.cell_);
}
template <class T>
bool Expression<T>::less(const Expression<T>& e) const {
  if (kind() < e.kind()) return true;
  if (e.kind() < kind()) return false;
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
  if (is_expanded()) return *this;
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
const std::map<Expression<T>, Expression<T>>& Expression<T>::base_to_exponent_map() const {
  SMATS_ASSERT(is_multiplication(), "Expression is not a multiplication");
  return cell_->template to<ExpressionMul>().base_to_exponent_map();
}

template <class T>
const Expression<T>& Expression<T>::lhs() const {
  switch (cell_->kind()) {
    case ExpressionKind::Pow:
      return cell_->template to<ExpressionPow>().lhs();
    case ExpressionKind::Div:
      return cell_->template to<ExpressionDiv>().lhs();
    default:
      SMATS_UNREACHABLE();
  }
}
template <class T>
const Expression<T>& Expression<T>::rhs() const {
  switch (cell_->kind()) {
    case ExpressionKind::Pow:
      return cell_->template to<ExpressionPow>().rhs();
    case ExpressionKind::Div:
      return cell_->template to<ExpressionDiv>().rhs();
    default:
      SMATS_UNREACHABLE();
  }
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
Expression<T>& Expression<T>::operator+=(const T& o) {
  // Simplification: 0 + E => E
  if (is_constant(0)) return *this = o;
  // Simplification: E + 0 => E
  // Simplification: E(c1) + E(c2) => E(c1 + c2)
  if (is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() += o;
      return *this;
    } else {
      return *this = Expression{constant() + o};
    }
  }
  ExpressionAddFactory<T> factory{is_addition() ? *this : o};
  factory += is_addition() ? o : *this;
  return *this = factory.build();
}
template <class T>
Expression<T>& Expression<T>::operator-=(const T& o) {
  if (is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() -= o;
      return *this;
    } else {
      return *this = Expression{constant() - o};
    }
  }
  return *this += -o;
}
template <class T>
Expression<T>& Expression<T>::operator*=(const T& o) {
  // Simplification: 1 * o => o
  if (is_constant(1)) return *this = o;
  // Simplification: 0 * o => 0
  if (is_constant(0)) return *this;
  // Simplification: E * 1 => E
  if (o == 1) return *this;
  // Simplification: E * 0 => 0
  if (o == 0) return *this = Expression::zero();
  // Simplification: (lhs / rhs) * o => (lhs * o) / rhs
  if (is_division()) return *this = (lhs() * o) / rhs();
  // Simplification: E(c1) * o => E(c1 * o)
  if (is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() *= o;
      return *this;
    } else {
      return *this = Expression{constant() * o};
    }
  }
  // Simplification: -1 * E => -E
  if (o == -1) {
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

  if (is_pow()) {
    const Expression& base = lhs();
    if (base.equal_to(o)) {
      // Simplification: pow(e1, e2) * e1 => pow(e1, e2 + 1)
      // TODO(soonho-tri): This simplification is not sound.
      const Expression& exponent = rhs();
      return *this = base ^ (exponent + 1);
    }
  }

  // Simplification: flattening
  ExpressionMulFactory<T> mul_factory{is_multiplication() ? *this : o};
  mul_factory *= is_multiplication() ? o : *this;
  return *this = mul_factory.build();
}
template <class T>
Expression<T>& Expression<T>::operator/=(const T& o) {
  // Simplification: E / 0 => NaN
  if (o == 0) return *this = Expression<T>::NaN();
  // Simplification: 0 / E => 0
  if (is_constant(0)) return *this;
  // Simplification: E / 1 => E
  if (o == 1) return *this;
  if (is_constant(o)) return *this = Expression<T>::one();
  // Simplification: E(c1) / E(c2) => E(c1 / c2)
  if (is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() /= o;
      return *this;
    } else {
      return *this = Expression{constant() / o};
    }
  }
  return *this *= Expression<T>{ExpressionDiv<T>::New(*this, o)};
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
  // Simplification: (E1 / E2) * (E3 / E4) => (E1 * E3) / (E2 * E4)
  if (is_division() && o.is_division()) {
    return *this = (lhs() * o.lhs()) / (rhs() * o.rhs());
  }
  // Simplification: (c / E) * rhs => (c * rhs) / E
  if (is_division() && lhs().is_constant()) {
    return *this = (lhs().constant() * o) / rhs();
  }
  // Simplification: lhs * (c / E) => (c * lhs) / E
  if (o.is_division() && o.lhs().is_constant()) {
    return *this = (o.lhs().constant() * lhs()) / o.rhs();
  }
  // Pow-related simplifications.
  if (is_pow()) {
    const Expression& base = lhs();
    if (o.is_pow()) {
      if (base.equal_to(o.lhs())) {
        // Simplification: pow(e1, e2) * pow(e1, e4) => pow(e1, e2 + e4)
        // TODO(soonho-tri): This simplification is not sound.
        //  For example, x^4 * x^(-3) => x
        //  The original expression `x^4 * x^(-3)` is evaluated to `nan` when x = 0
        //  while the simplified expression `x` is evaluated to 0.
        return *this = base ^ (rhs() + o.rhs());
      }
    }
    if (base.equal_to(o)) {
      // Simplification: pow(e1, e2) * e1 => pow(e1, e2 + 1)
      // TODO(soonho-tri): This simplification is not sound.
      return *this = base ^ (rhs() + 1);
    }
  } else {
    if (o.is_pow()) {
      const Expression& base = o.lhs();
      if (base.equal_to(*this)) {
        // Simplification: (lhs * rhs == e1 * pow(e1, e2)) => pow(e1, 1 + e2)
        // TODO(soonho-tri): This simplification is not sound.
        return *this = base ^ (o.rhs() + 1);
      }
    }
  }
  if (!is_multiplication() && !o.is_multiplication() && equal_to(o)) {
    // Simplification: x * x => x^2 (=pow(x,2))
    return *this = lhs() ^ 2;
  }
  // Simplification: flattening
  ExpressionMulFactory<T> mul_factory{is_multiplication() ? *this : o};
  mul_factory *= is_multiplication() ? o : *this;
  return *this = mul_factory.build();
}
template <class T>
Expression<T>& Expression<T>::operator/=(const Expression<T>& o) {
  // Simplification: E / 0 => NaN
  if (o.is_constant(0)) return *this = Expression::NaN();
  // Simplification: E / 1 => x
  if (o.is_constant(1)) return *this;
  // Simplification: 0 / E => 0
  if (is_constant(0)) return *this;
  if (equal_to(o)) return *this = Expression<T>::one();
  // Simplification: E(c1) / E(c2) => E(c1 / c2)
  if (is_constant() && o.is_constant()) {
    if (cell_.use_count() == 1) {
      m_cell().template to<ExpressionConstant>().m_value() /= o.constant();
      return *this;
    } else {
      return *this = Expression{constant() / o.constant()};
    }
  }
  return *this *= Expression<T>{ExpressionDiv<T>::New(*this, o)};
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
  if (is_pow() && rhs().is_constant() && o.is_constant()) {
    // pow(base, exponent) ^ e2 => pow(base, exponent * e2) only if both of exponent and e2 are integers.
    const T& pow_exponent_value = o.rhs().constant();
    const T& exponent_value = o.constant();
    if (is_integer(pow_exponent_value) && is_integer(exponent_value)) {
      return *this = Expression{ExpressionPow<T>::New(lhs(), Expression<T>{pow_exponent_value * exponent_value})};
    }
  }
  return *this = Expression<T>{ExpressionPow<T>::New(*this, o)};
}
template <class T>
Expression<T> Expression<T>::operator^(const Expression<T>& o) const {
  Expression tmp{*this};
  tmp ^= o;
  return tmp;
}

template <class T>
Expression<T> operator+(const T& c, const Expression<T>& e) {
  return e + c;
}
template <class T>
Expression<T> operator-(const T& c, const Expression<T>& e) {
  return Expression<T>{c} -= e;
}
template <class T>
Expression<T> operator*(const T& c, const Expression<T>& e) {
  return e * c;
}
template <class T>
Expression<T> operator/(const T& c, const Expression<T>& e) {
  return Expression<T>{c} /= e;
}
template <class T>
Expression<T> operator^(const T& c, const Expression<T>& e) {
  return Expression<T>{c} ^= e;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const Expression<T>& expr) {
  return os << expr.to_string();
}

EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(Expression);
template Expression<int> operator+(const int& c, const Expression<int>& e);
template Expression<long> operator+(const long& c, const Expression<long>& e);
template Expression<double> operator+(const double& c, const Expression<double>& e);
template Expression<float> operator+(const float& c, const Expression<float>& e);
template Expression<int> operator-(const int& c, const Expression<int>& e);
template Expression<long> operator-(const long& c, const Expression<long>& e);
template Expression<double> operator-(const double& c, const Expression<double>& e);
template Expression<float> operator-(const float& c, const Expression<float>& e);
template Expression<int> operator*(const int& c, const Expression<int>& e);
template Expression<long> operator*(const long& c, const Expression<long>& e);
template Expression<double> operator*(const double& c, const Expression<double>& e);
template Expression<float> operator*(const float& c, const Expression<float>& e);
template Expression<int> operator/(const int& c, const Expression<int>& e);
template Expression<long> operator/(const long& c, const Expression<long>& e);
template Expression<double> operator/(const double& c, const Expression<double>& e);
template Expression<float> operator/(const float& c, const Expression<float>& e);
template Expression<int> operator^(const int& c, const Expression<int>& e);
template Expression<long> operator^(const long& c, const Expression<long>& e);
template Expression<double> operator^(const double& c, const Expression<double>& e);
template Expression<float> operator^(const float& c, const Expression<float>& e);

template std::ostream& operator<<(std::ostream& os, const Expression<int>& expr);
template std::ostream& operator<<(std::ostream& os, const Expression<long>& expr);
template std::ostream& operator<<(std::ostream& os, const Expression<double>& expr);
template std::ostream& operator<<(std::ostream& os, const Expression<float>& expr);

}  // namespace smats
