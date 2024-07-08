/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/expression_cell.h"

#include <numeric>

#include "smats/util/exception.h"

namespace smats {

/**
 * ExpressionCell
 */
template <class T>
ExpressionCell<T>::ExpressionCell(const ExpressionKind kind, const bool is_polynomial, const bool is_expanded)
    : kind_{kind}, is_polynomial_{is_polynomial}, is_expanded_{is_expanded} {
  std::cout << "ExpressionCell constructor" << std::endl;
  std::cout << "kind: " << kind << std::endl;
  std::cout << "is_polynomial: " << is_polynomial << std::endl;
  std::cout << "is_expanded: " << is_expanded << std::endl;
}

/**
 * UnaryExpressionCell
 */
template <class T>
UnaryExpressionCell<T>::UnaryExpressionCell(ExpressionKind kind, Expression<T> e, bool is_polynomial, bool is_expanded)
    : ExpressionCell<T>{kind, is_polynomial, is_expanded}, e_{e} {}
template <class T>
void UnaryExpressionCell<T>::hash(DelegatingHasher& hasher) const {
  return e_.hash(hasher);
}
template <class T>
Variables UnaryExpressionCell<T>::variables() const {
  return e_.variables();
}
template <class T>
bool UnaryExpressionCell<T>::equal_to(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == UnaryExpressionCell<T>::kind(), "Expressions must have the same kind");
  const auto& unary_e = static_cast<const UnaryExpressionCell&>(e);
  return e_.equal_to(unary_e.e_);
}
template <class T>
bool UnaryExpressionCell<T>::less(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == UnaryExpressionCell<T>::kind(), "Expressions must have the same kind");
  const auto& unary_e = static_cast<const UnaryExpressionCell&>(e);
  return e_.less(unary_e.e_);
}
template <class T>
T UnaryExpressionCell<T>::evaluate(const Environment<T>& env) const {
  return e_.evaluate(env);
}

/**
 * BinaryExpressionCell
 */
template <class T>
BinaryExpressionCell<T>::BinaryExpressionCell(ExpressionKind kind, Expression<T> e1, Expression<T> e2,
                                              bool is_polynomial, bool is_expanded)
    : ExpressionCell<T>{kind, is_polynomial, is_expanded}, e1_{e1}, e2_{e2} {}
template <class T>
void BinaryExpressionCell<T>::hash(DelegatingHasher& hasher) const {
  e1_.hash(hasher);
  e2_.hash(hasher);
}
template <class T>
Variables BinaryExpressionCell<T>::variables() const {
  return e1_.variables() + e2_.variables();
}
template <class T>
bool BinaryExpressionCell<T>::equal_to(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == BinaryExpressionCell<T>::kind(), "Expressions must have the same kind");
  const auto& binary_e = static_cast<const BinaryExpressionCell&>(e);
  return e1_.equal_to(binary_e.e1_) && e2_.equal_to(binary_e.e2_);
}
template <class T>
bool BinaryExpressionCell<T>::less(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == BinaryExpressionCell<T>::kind(), "Expressions must have the same kind");
  const auto& binary_e = static_cast<const BinaryExpressionCell&>(e);
  return e1_.less(binary_e.e1_) || (e1_.equal_to(binary_e.e1_) && e2_.less(binary_e.e2_));
}
template <class T>
T BinaryExpressionCell<T>::evaluate(const Environment<T>& env) const {
  // TODO(ernesto): Use evaluate_impl
  return (e1_.evaluate(env) + e2_.evaluate(env));
}

/**
 * ExpressionConstant
 */
template <class T>
ExpressionConstant<T>::ExpressionConstant(ExpressionCell<T>::Private, const T& value)
    : ExpressionCell<T>{ExpressionKind::Constant, true, true}, value_{value} {}
template <class T>
void ExpressionConstant<T>::hash(DelegatingHasher& hasher) const {
  hash_append(hasher, value_);
}
template <class T>
Variables ExpressionConstant<T>::variables() const {
  return Variables{};
}
template <class T>
bool ExpressionConstant<T>::equal_to(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionConstant<T>::kind(), "Expressions must have the same kind");
  const auto& constant_e = static_cast<const ExpressionConstant&>(e);
  return value_ == constant_e.value_;
}
template <class T>
bool ExpressionConstant<T>::less(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionConstant<T>::kind(), "Expressions must have the same kind");
  const auto& constant_e = static_cast<const ExpressionConstant&>(e);
  return value_ < constant_e.value_;
}
template <class T>
T ExpressionConstant<T>::evaluate(const Environment<T>& env) const {
  return value_;
}
template <class T>
Expression<T> ExpressionConstant<T>::expand() const {
  return ExpressionCell<T>::to_expression();
}
template <class T>
Expression<T> ExpressionConstant<T>::evaluate_partial(const Environment<T>& env) const {
  return ExpressionCell<T>::to_expression();
}
template <class T>
Expression<T> ExpressionConstant<T>::substitute(const Substitution<T>& s) const {
  return ExpressionCell<T>::to_expression();
}
template <class T>
Expression<T> ExpressionConstant<T>::differentiate(const Variable& x) const {
  return Expression<T>::zero();
}
template <class T>
std::ostream& ExpressionConstant<T>::display(std::ostream& os) const {
  return os << value_;
}

/**
 * ExpressionVar
 */
template <class T>
ExpressionVar<T>::ExpressionVar(ExpressionCell<T>::Private, Variable var)
    : ExpressionCell<T>{ExpressionKind::Var, true, true}, var_{std::move(var)} {}
template <class T>
void ExpressionVar<T>::hash(DelegatingHasher& hasher) const {
  hash_append(hasher, var_);
}
template <class T>
Variables ExpressionVar<T>::variables() const {
  return Variables{var_};
}
template <class T>
bool ExpressionVar<T>::equal_to(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionVar<T>::kind(), "Expressions must have the same kind");
  const auto& var_e = static_cast<const ExpressionVar&>(e);
  return var_.equal_to(var_e.var_);
}
template <class T>
bool ExpressionVar<T>::less(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionVar<T>::kind(), "Expressions must have the same kind");
  const auto& var_e = static_cast<const ExpressionVar&>(e);
  return var_.less(var_e.var_);
}
template <class T>
T ExpressionVar<T>::evaluate(const Environment<T>& env) const {
  return env.at(var_);
}
template <class T>
Expression<T> ExpressionVar<T>::expand() const {
  return ExpressionCell<T>::to_expression();
}
template <class T>
Expression<T> ExpressionVar<T>::evaluate_partial(const Environment<T>& env) const {
  auto it = env.find(var_);
  return it != env.end() ? Expression<T>{it->second} : ExpressionCell<T>::to_expression();
}
template <class T>
Expression<T> ExpressionVar<T>::substitute(const Substitution<T>& s) const {
  auto it = s.find(var_);
  return it != s.end() ? Expression<T>{it->second} : ExpressionCell<T>::to_expression();
}
template <class T>
Expression<T> ExpressionVar<T>::differentiate(const Variable& x) const {
  return var_.equal_to(x) ? Expression<T>::one() : Expression<T>::zero();
}
template <class T>
std::ostream& ExpressionVar<T>::display(std::ostream& os) const {
  return os << var_;
}

/*
 * ExpressionNaN
 */
template <class T>
ExpressionNaN<T>::ExpressionNaN(ExpressionCell<T>::Private) : ExpressionCell<T>{ExpressionKind::NaN, false, false} {}
template <class T>
void ExpressionNaN<T>::hash(DelegatingHasher& hasher) const {
  SMATS_RUNTIME_ERROR("Cannot compute hash of NaN expression");
}
template <class T>
Variables ExpressionNaN<T>::variables() const {
  return Variables{};
}
template <class T>
bool ExpressionNaN<T>::equal_to(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionNaN<T>::kind(), "Expressions must have the same kind");
  return false;
}
template <class T>
bool ExpressionNaN<T>::less(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionNaN<T>::kind(), "Expressions must have the same kind");
  return false;
}
template <class T>
T ExpressionNaN<T>::evaluate(const Environment<T>& env) const {
  SMATS_RUNTIME_ERROR("Cannot evaluate NaN expression");
}
template <class T>
Expression<T> ExpressionNaN<T>::expand() const {
  SMATS_RUNTIME_ERROR("Cannot expand NaN expression");
}
template <class T>
Expression<T> ExpressionNaN<T>::evaluate_partial(const Environment<T>& env) const {
  SMATS_RUNTIME_ERROR("Cannot evaluate NaN expression");
}
template <class T>
Expression<T> ExpressionNaN<T>::substitute(const Substitution<T>& s) const {
  SMATS_RUNTIME_ERROR("Cannot substitute NaN expression");
}
template <class T>
Expression<T> ExpressionNaN<T>::differentiate(const Variable& x) const {
  SMATS_RUNTIME_ERROR("Cannot differentiate NaN expression");
}
template <class T>
std::ostream& ExpressionNaN<T>::display(std::ostream& os) const {
  return os << "NaN";
}

/*
 * ExpressionAdd
 */
template <class T>
ExpressionAdd<T>::ExpressionAdd(ExpressionCell<T>::Private, T constant, ExpressionMap expr_to_coeff_map)
    : ExpressionCell<T>{ExpressionKind::Add, true, true},
      constant_{std::move(constant)},
      expr_to_coeff_map_{std::move(expr_to_coeff_map)} {}
template <class T>
void ExpressionAdd<T>::hash(smats::DelegatingHasher& hasher) const {
  hash_append(hasher, constant_);
  for (const auto& [expr, coeff] : expr_to_coeff_map_) {
    expr.hash(hasher);
    hash_append(hasher, coeff);
  }
}
template <class T>
Variables ExpressionAdd<T>::variables() const {
  Variables vars;
  for (const auto& [expr, _] : expr_to_coeff_map_) vars += expr.variables();
  return vars;
}
template <class T>
bool ExpressionAdd<T>::equal_to(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionAdd<T>::kind(), "Expressions must have the same kind");
  const auto& add_e = static_cast<const ExpressionAdd&>(o);
  if (constant_ != add_e.constant_ || expr_to_coeff_map_.size() != add_e.expr_to_coeff_map_.size()) return false;
  return std::all_of(expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(), [&add_e](const auto& p) {
    auto it = add_e.expr_to_coeff_map_.find(p.first);
    return it != add_e.expr_to_coeff_map_.end() && it->second == p.second;
  });
}
template <class T>
bool ExpressionAdd<T>::less(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionAdd<T>::kind(), "Expressions must have the same kind");
  const auto& add_e = static_cast<const ExpressionAdd&>(o);
  if (constant_ < add_e.constant_) return true;
  if (add_e.constant_ < constant_) return false;

  return lexicographical_compare(
      expr_to_coeff_map_.cbegin(), expr_to_coeff_map_.cend(), add_e.expr_to_coeff_map_.cbegin(),
      add_e.expr_to_coeff_map_.cend(),
      [](const std::pair<const Expression<T>, T>& p1, const std::pair<const Expression<T>, T>& p2) {
        if (p1.first.less(p1.first)) return true;
        if (p1.first.less(p1.first)) return false;
        return p1.second < p2.second;
      });
}
template <class T>
T ExpressionAdd<T>::evaluate(const Environment<T>& env) const {
  return std::reduce(expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(), constant_,
                     [&env](T acc, const auto& p) { return acc + p.second * p.first.evaluate(env); });
}
template <class T>
Expression<T> ExpressionAdd<T>::expand() const {
  Expression<T> result{constant_};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += Expression{coeff * expr}.expand();
  return result;
}
template <class T>
Expression<T> ExpressionAdd<T>::evaluate_partial(const Environment<T>& env) const {
  Expression<T> result{constant_};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += coeff * Expression{expr}.evaluate_partial(env);
  return result;
}
template <class T>
Expression<T> ExpressionAdd<T>::substitute(const Substitution<T>& s) const {
  Expression<T> result{constant_};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += coeff * Expression{expr}.substitute(s);
  return result;
}
template <class T>
Expression<T> ExpressionAdd<T>::differentiate(const Variable& x) const {
  Expression<T> result{0};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += coeff * Expression{expr}.differentiate(x);
  return result;
}
template <class T>
std::ostream& ExpressionAdd<T>::display(std::ostream& os) const {
  SMATS_ASSERT(!expr_to_coeff_map_.empty(), "ExpressionAdd must have at least one term");
  bool print_plus = false;
  os << "(";
  if (constant_ != 0.0) {
    os << constant_;
    print_plus = true;
  }
  for (auto& p : expr_to_coeff_map_) {
    display_term(os, print_plus, p.second, p.first);
    print_plus = true;
  }
  os << ")";
  return os;
}
template <class T>
std::ostream& ExpressionAdd<T>::display_term(std::ostream& os, const bool print_plus, const T& coeff,
                                             const Expression<T>& term) const {
  SMATS_ASSERT(coeff != 0.0, "Coefficient must be non-zero");
  if (coeff > 0.0) {
    if (print_plus) os << " + ";
    // Do not print "1 * t"
    if (coeff != 1.0) os << coeff << " * ";
  } else {
    // Instead of printing "+ (- E)", just print "- E".
    os << " - ";
    if (coeff != -1.0) os << (-coeff) << " * ";
  }
  return os << term;
}

EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionCell);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(UnaryExpressionCell);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(BinaryExpressionCell);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionConstant);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionVar);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionNaN);

}  // namespace smats
