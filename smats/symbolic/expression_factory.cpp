/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/expression_factory.h"

#include <utility>

#include "smats/symbolic/expression_cell.h"
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
  if (e.kind() == ExpressionKind::Constant) {
    constant_ = e.template to<ExpressionConstant>().value();
  } else if (e.kind() == ExpressionKind::Add) {
    constant_ = e.template to<ExpressionAdd>().constant();
    expr_to_coeff_map_ = e.template to<ExpressionAdd>().expr_to_coeff_map();
  } else {
    expr_to_coeff_map_[Expression<T>{e.ptr()}] = 1;
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
  SMATS_ASSERT(o.cell() != nullptr, "ExpressionAddFactory::operator+=: nullptr");
  return *this += *o.cell();
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
    case ExpressionKind::Add /:
      constant_ += o.template to<ExpressionAdd>().constant();
      for (const auto &[e, c] : o.template to<ExpressionAdd>().expr_to_coeff_map()) {
        expr_to_coeff_map_[e] += c;
      }
      break;
    case ExpressionKind::Mul:
      const T &constant = o.constant_value();
      if (constant != 1.0) {
        SMATS_UNREACHABLE();
        // Instead of adding (1.0 * (constant * b1^t1 ... bn^tn)),
        // add (constant, 1.0 * b1^t1 ... bn^tn).
        //      return add(constant, ExpressionMulFactory(1.0,
        //      get_base_to_exponent_map_in_multiplication(e)).GetExpression());
      }
      break;
    default:
      expr_to_coeff_map_[Expression<T>{o.ptr()}] += 1;
      break;
  }
  return *this;
}
template <classT T>
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
      return std::shared_ptr<const ExpressionCell<T>>{ExpressionVar<T>::New({})};  // TODO(ernesto): implement mul
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
ExpressionMulFactory<T>::ExpressionMulFactory(const ExpressionCell<T> &e) : constant_{1.0}, base_to_exponent_map_{} {
  SMATS_ASSERT(
      e.kind() == ExpressionKind::Mul || e.kind() == ExpressionKind::Constant || e.kind() == ExpressionKind::Var,
      "ExpressionMulFactory::ExpressionMulFactory: invalid kind");
  if (e.kind() == ExpressionKind::Constant) {
    constant_ = e.template to<ExpressionConstant>().value();
  } else if (e.kind() == ExpressionKind::Var) {
    base_to_exponent_map_[Expression<T>{e.ptr()}] = Expression<T>::one();
  } else {
    constant_ = e.template to<ExpressionMul>().constant();
    base_to_exponent_map_ = e.template to<ExpressionMul>().base_to_exponent_map();
  }
}

EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionAddFactory);

}  // namespace smats
