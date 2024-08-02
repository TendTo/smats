/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/expression_factory.h"

#include <utility>

#include "smats/symbolic/expression_cell.h"
#include "smats/symbolic/symbolic_util.h"
#include "smats/util/error.h"

namespace smats {

/**
 * ExpressionAddFactory
 */
template <class T>
ExpressionAddFactory<T>::ExpressionAddFactory(T constant, std::map<Expression<T>, T> expr_to_coeff_map)
    : constant_{std::move(constant)}, expr_to_coeff_map_{std::move(expr_to_coeff_map)} {}
template <class T>
ExpressionAddFactory<T>::ExpressionAddFactory(const Expression<T> &e) : ExpressionAddFactory{e.cell()} {}
template <class T>
ExpressionAddFactory<T>::ExpressionAddFactory(const std::shared_ptr<const ExpressionCell<T>> &e)
    : ExpressionAddFactory(*e) {}
template <class T>
ExpressionAddFactory<T>::ExpressionAddFactory(const ExpressionCell<T> &e) : constant_{}, expr_to_coeff_map_{} {
  switch (e.kind()) {
    case ExpressionKind::Constant:
      constant_ = e.template to<ExpressionConstant>().value();
      break;
    case ExpressionKind::Add:
      constant_ = e.template to<ExpressionAdd>().constant();
      expr_to_coeff_map_ = e.template to<ExpressionAdd>().expr_to_coeff_map();
      break;
    default:
      expr_to_coeff_map_[e.to_expression()] = 1;
      break;
  }
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::operator+=(const ExpressionAddFactory<T> &o) {
  constant_ += o.constant_;
  for (const auto &[e, c] : o.expr_to_coeff_map_) expr_to_coeff_map_[e] += c;
  return *this;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::operator+=(const T &o) {
  constant_ += o;
  return *this;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::operator+=(const Expression<T> &o) {
  SMATS_ASSERT(o.cell_ != nullptr, "ExpressionAddFactory::operator+=: nullptr");
  return *this += o.cell();
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::operator+=(const std::shared_ptr<ExpressionCell<T>> &o) {
  SMATS_ASSERT(o != nullptr, "ExpressionAddFactory::operator+=: nullptr");
  return *this += *o;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::operator+=(const ExpressionCell<T> &o) {
  switch (o.kind()) {
    case ExpressionKind::Constant:
      constant_ += o.template to<ExpressionConstant>().value();
      break;
    case ExpressionKind::Add:
      constant_ += o.template to<ExpressionAdd>().constant();
      for (const auto &[e, c] : o.template to<ExpressionAdd>().expr_to_coeff_map()) {
        expr_to_coeff_map_[e] += c;
      }
      break;
    case ExpressionKind::Mul:
      if (o.template to<ExpressionMul>().constant() != 1.0) {
        SMATS_UNREACHABLE();
        // Instead of adding (1.0 * (constant * b1^t1 ... bn^tn)),
        // add (constant, 1.0 * b1^t1 ... bn^tn).
        //      return add(constant, ExpressionMulFactory(1.0,
        //      get_base_to_exponent_map_in_multiplication(e)).GetExpression());
      }
      break;
    default:
      expr_to_coeff_map_[o.to_expression()] += 1;
      break;
  }
  return *this;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::add(const ExpressionAddFactory<T> &o) {
  return *this += o;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::add(const T &o) {
  return *this += o;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::add(const Expression<T> &o) {
  return *this += o;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::add(const std::shared_ptr<ExpressionCell<T>> &o) {
  return *this += o;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::add(const ExpressionCell<T> &o) {
  return *this += o;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::add(const T &constant,
                                                      const std::map<Expression<T>, T> &expr_to_coeff_map) {
  constant_ += constant;
  for (const auto &[e, c] : expr_to_coeff_map) expr_to_coeff_map_[e] += c;
  return *this;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::add(const T &coeff, const Expression<T> &expr) {
  expr_to_coeff_map_[expr] += coeff;
  return *this;
}
template <class T>
ExpressionAddFactory<T> &ExpressionAddFactory<T>::negate() {
  constant_ = -constant_;
  for (auto &[e, c] : expr_to_coeff_map_) c = -c;
  return *this;
}
template <class T>
Expression<T> ExpressionAddFactory<T>::build() const {
  if (constant_ == 0 && expr_to_coeff_map_.empty()) return Expression<T>::zero();
  if (constant_ == 0 && expr_to_coeff_map_.size() == 1) {
    const auto &[e, c] = *expr_to_coeff_map_.begin();
    if (c == 1)
      return e;
    else
      return Expression<T>{ExpressionMul<T>::New(c, {{e, 1}})};
  }
  return Expression<T>{ExpressionAdd<T>::New(constant_, expr_to_coeff_map_)};
}

/**
 * ExpressionMulFactory
 */
template <class T>
ExpressionMulFactory<T>::ExpressionMulFactory(T constant, std::map<Expression<T>, Expression<T>> expr_to_coeff_map)
    : constant_{std::move(constant)}, base_to_exponent_map_{std::move(expr_to_coeff_map)} {}
template <class T>
ExpressionMulFactory<T>::ExpressionMulFactory(const Expression<T> &e) : ExpressionMulFactory{e.cell()} {}
template <class T>
ExpressionMulFactory<T>::ExpressionMulFactory(const std::shared_ptr<const ExpressionCell<T>> &e)
    : ExpressionMulFactory{*e} {}
template <class T>
ExpressionMulFactory<T>::ExpressionMulFactory(const ExpressionCell<T> &e) : constant_{1}, base_to_exponent_map_{} {
  switch (e.kind()) {
    case ExpressionKind::Constant:
      constant_ = e.template to<ExpressionConstant>().value();
      break;
    case ExpressionKind::Mul:
      constant_ = e.template to<ExpressionMul>().constant();
      base_to_exponent_map_ = e.template to<ExpressionMul>().base_to_exponent_map();
      break;
    case ExpressionKind::Pow:
      base_to_exponent_map_[e.template to<ExpressionPow>().lhs()] = e.template to<ExpressionPow>().rhs();
      break;
    default:
      base_to_exponent_map_.emplace(e.to_expression(), Expression<T>::one());
      break;
  }
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::operator*=(const T &o) {
  if (constant_ == 0 || o == 1) return *this;
  if (o == 0) {
    constant_ = 0;
    base_to_exponent_map_.clear();
    return *this;
  }
  constant_ *= o;
  return *this;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::operator*=(const Expression<T> &o) {
  SMATS_ASSERT(o.cell_ != nullptr, "ExpressionMulFactory::operator*=: nullptr");
  return *this *= o.cell();
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::operator*=(const std::shared_ptr<ExpressionCell<T>> &o) {
  SMATS_ASSERT(o != nullptr, "ExpressionMulFactory::operator*=: nullptr");
  return *this *= *o;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::operator*=(const ExpressionCell<T> &o) {
  if (constant_ == 0) return *this;
  switch (o.kind()) {
    case ExpressionKind::Constant:
      if (o.template to<ExpressionConstant>().value() == 0) {
        constant_ = 0;
        base_to_exponent_map_.clear();
        return *this;
      }
      constant_ *= o.template to<ExpressionConstant>().value();
      return *this;
    case ExpressionKind::Mul:
      constant_ *= o.template to<ExpressionMul>().constant();
      for (const auto &[b, e] : o.template to<ExpressionMul>().base_to_exponent_map()) {
        base_to_exponent_map_[b] += e;
      }
      return *this;
    case ExpressionKind::Pow:
      return multiply(o.template to<ExpressionPow>().lhs(), o.template to<ExpressionPow>().rhs());
    default:
      base_to_exponent_map_[o.to_expression()] += Expression<T>::one();
      return *this;
  }
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::multiply(const T &o) {
  return *this *= o;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::multiply(const Expression<T> &o) {
  return *this *= o;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::multiply(const std::shared_ptr<ExpressionCell<T>> &o) {
  return *this *= o;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::multiply(const ExpressionCell<T> &o) {
  return *this *= o;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::multiply(
    const T &constant, const std::map<Expression<T>, Expression<T>> &base_to_exponent_map) {
  if (constant == 0) {
    constant_ = 0;
    base_to_exponent_map_.clear();
    return *this;
  }
  constant_ *= constant;
  for (const auto &[b, e] : base_to_exponent_map) base_to_exponent_map_[b] += e;
  return *this;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::multiply(const Expression<T> &base, const Expression<T> &exponent) {
  // The following assertion holds because of ExpressionMulFactory::AddExpression.
  if (base.is_pow() && exponent.is_constant()) {
    const Expression<T> &base_exponent = base.rhs();
    if (base_exponent.is_constant()) {
      const T &base_exponent_value = base_exponent.constant();
      if (is_integer(base_exponent_value)) {
        // If base = pow(e1, e2) and both of e2 and exponent are integers, add (e1, e2 * exponent).
        // Example: (x^2)^3 => x^(2 * 3)
        return multiply(base.lhs(), base_exponent_value * exponent);
      }
    }
  }

  const auto it(base_to_exponent_map_.find(base));
  if (it != base_to_exponent_map_.end()) {
    // base is already in map.
    // (= b1^e1 * ... * (base^new_exponent) * ... * en^bn).
    // Update it to be (... * (base^(new_exponent + exponent)) * ...)
    // Example: x^3 * x^2 => x^5
    Expression<T> &new_exponent = it->second;
    new_exponent += exponent;
    if (new_exponent.is_constant(0)) {
      // If it ends up with base^0 (= 1.0) then remove this entry from the map.
      // TODO(soonho-tri): The following operation is not sound
      //  since it can cancels `base` which might include 0/0 problems.
      base_to_exponent_map_.erase(it);
    }
  } else {
    // Product is not found in base_to_exponent_map_. Add the entry (base, exponent).
    base_to_exponent_map_.emplace(base, exponent);
  }
  return *this;
}
template <class T>
ExpressionMulFactory<T> &ExpressionMulFactory<T>::negate() {
  constant_ = -constant_;
  return *this;
}
template <class T>
Expression<T> ExpressionMulFactory<T>::build() const {
  if (constant_ == 0) return Expression<T>::zero();
  if (base_to_exponent_map_.empty()) return Expression<T>{constant_};
  if (constant_ == 1 && base_to_exponent_map_.size() == 1u) {
    const auto &[base, exponent] = *base_to_exponent_map_.begin();
    return exponent.is_constant(1) ? base : Expression<T>{ExpressionPow<T>::New(base, exponent)};
  }
  return Expression<T>{ExpressionMul<T>::New(constant_, base_to_exponent_map_)};
}

EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionAddFactory);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionMulFactory);

}  // namespace smats
