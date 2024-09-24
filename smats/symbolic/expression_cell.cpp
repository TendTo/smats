/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/expression_cell.h"

#include <limits>
#include <numeric>

#include "smats/symbolic/expression_factory.h"
#include "smats/symbolic/symbolic_util.h"
#include "smats/util/error.h"

namespace smats {

namespace {

// Helper function expanding (e1 * e2). It assumes that both of e1 and e2 are
// already expanded.
template <class T>
Expression<T> expand_multiplication(const Expression<T>& e1, const Expression<T>& e2) {
  // Precondition: e1 and e2 are already expanded.
  SMATS_ASSERT(e1.equal_to(e1.expand()), "e1 must be expanded");
  SMATS_ASSERT(e2.equal_to(e2.expand()), "e2 must be expanded");

  if (e1.is_addition()) {
    //   (c0 + c1 * e_{1,1} + ... + c_n * e_{1, n}) * e2
    // = c0 * e2 + c1 * e_{1,1} * e2 + ... + c_n * e_{1,n} * e2
    Expression<T> ret{expand_multiplication(Expression<T>{e1.constant()}, e2)};
    for (const auto& [exp, coeff] : e1.expression_to_coeff_map()) {
      ret += expand_multiplication(expand_multiplication(Expression<T>{coeff}, exp), e2);
    }
    return ret;
  }
  if (e2.is_addition()) {
    //   e1 * (c0 + c1 * e_{2,1} + ... + c_n * e_{2, n})
    // = e1 * c0 + e1 * c1 * e_{2,1} + ... + e1 * c_n * e_{2,n}
    Expression<T> ret{expand_multiplication(e1, Expression<T>{e2.constant()})};
    for (const auto& [exp, coeff] : e2.expression_to_coeff_map()) {
      ret += expand_multiplication(expand_multiplication(e1, Expression<T>{coeff}), exp);
    }
    return ret;
  }
#if 0
  if (e1.is_division()) {
    const Expression<T>& e1_1{get_first_argument(e1)};
    const Expression<T>& e1_2{get_second_argument(e1)};
    if (e2.is_division()) {
      //    ((e1_1 / e1_2) * (e2_1 / e2_2)).Expand()
      // => (e1_1 * e2_1).Expand() / (e1_2 * e2_2).Expand().
      //
      // Note that e1_1, e1_2, e2_1, and e_2 are already expanded by the
      // precondition.
      const Expression& e2_1{get_first_argument(e2)};
      const Expression& e2_2{get_second_argument(e2)};
      return ExpandMultiplication(e1_1, e2_1) / ExpandMultiplication(e1_2, e2_2);
    }
    //    ((e1_1 / e1_2) * e2).Expand()
    // => (e1_1 * e2).Expand() / e2.
    //
    // Note that e1_1, e1_2, and e_2 are already expanded by the precondition.
    return ExpandMultiplication(e1_1, e2) / e1_2;
  }
  if (e2.is_division()) {
    //    (e1 * (e2_1 / e2_2)).Expand()
    // => (e1 * e2_1).Expand() / e2_2.
    //
    // Note that e1, e2_1, and e2_2 are already expanded by the precondition.
    const Expression& e2_1{get_first_argument(e2)};
    const Expression& e2_2{get_second_argument(e2)};
    return expand_multiplication(e1, e2_1) / e2_2;
  }
#endif
  return e1 * e2;
}

/**
 * Helper function expanding @f$pow(base, n)@f$.
 *
 * It assumes that base is already expanded and that n is a positive integer.
 * @pre @p base is already expanded
 * @pre @p n is a positive integer
 * @tparam T type of the expression evaluation
 * @param base base of the exponentiation
 * @param n exponent
 * @return expanded expression
 */
template <class T>
Expression<T> expand_pow(const Expression<T>& base, const int n) {
  SMATS_ASSERT(base.equal_to(base.expand()), "base must be expanded");
  SMATS_ASSERT(n >= 1, "n must be greater than or equal to 1");
  // pow(base, 0) = 1
  if (n == 0) return Expression<T>::one();
  // pow(base, 1) = base
  if (n == 1) return base;
  const Expression pow_half{expand_pow(base, n / 2)};
  // If n is odd, then
  // pow(base, n) = base * pow(base, n / 2) * pow(base, n / 2)
  // else
  // pow(base, n) = pow(base, n / 2) * pow(base, n / 2)
  return (n % 2 == 1) ? expand_multiplication(expand_multiplication(base, pow_half), pow_half)
                      : expand_multiplication(pow_half, pow_half);
}

/**
 * Helper function expanding @f$pow(base, exponent)@f$.
 *
 * It assumes that both base and exponent are already expanded.
 * @pre @p base is already expanded
 * @pre @p exponent is already expanded
 * @tparam T type of the expression evaluation
 * @param base base of the exponentiation
 * @param exponent exponent
 * @return expanded expression
 */
template <class T>
Expression<T> expand_pow(const Expression<T>& base, const Expression<T>& exponent) {
  SMATS_ASSERT(base.equal_to(base.expand()), "base must be expanded");
  SMATS_ASSERT(exponent.equal_to(exponent.expand()), "exponent must be expanded");
  if (base.is_multiplication()) {
    //   pow(c * ∏ᵢ pow(e₁ᵢ, e₂ᵢ), exponent)
    // = pow(c, exponent) * ∏ᵢ pow(e₁ᵢ, e₂ᵢ * exponent)
    const T& c = base.constant();
    std::map<smats::Expression<T>, smats::Expression<T>> base_to_exponent_map{base.base_to_exponent_map()};
    for (std::pair<const Expression<T>, Expression<T>>& p : base_to_exponent_map) {
      p.second = p.second * exponent;
    }
    return (c ^ exponent) * ExpressionMulFactory<T>{1, std::move(base_to_exponent_map)}.consume();
  }

  // Expand if
  //     1) base is an addition expression and
  //     2) exponent is a positive integer.
  if (!base.is_addition() || !exponent.is_constant()) return base ^ exponent;

  const T e{exponent.constant()};
  if (e <= 0 || !is_integer(e)) return base ^ exponent;

  const int n{static_cast<int>(e)};
  return expand_pow(base, n);
}
}  // namespace

/**
 * ExpressionCell
 */
template <class T>
ExpressionCell<T>::ExpressionCell(const ExpressionKind kind, const bool is_expanded)
    : kind_{kind}, variables_{std::nullopt}, is_polynomial_{std::nullopt}, is_expanded_{is_expanded} {}
template <class T>
ExpressionCell<T>::ExpressionCell(const ExpressionKind kind, const bool is_polynomial, const bool is_expanded)
    : kind_{kind}, variables_{std::nullopt}, is_polynomial_{is_polynomial}, is_expanded_{is_expanded} {}
template <class T>
const Variables& ExpressionCell<T>::variables() const {
  if (!variables_.has_value()) compute_variables(variables_);
  SMATS_ASSERT(variables_.has_value(), "variables_ must have a value by now");
  return variables_.value();
}
template <class T>
bool ExpressionCell<T>::is_polynomial() const {
  if (!is_polynomial_.has_value()) compute_is_polynomial(is_polynomial_);
  SMATS_ASSERT(is_polynomial_.has_value(), "is_polynomial_ must have a value by now");
  return is_polynomial_.value();
}
template <class T>
void ExpressionCell<T>::invalidate_cache() {
  variables_.reset();
  is_polynomial_.reset();
}

/**
 * UnaryExpressionCell
 */
template <class T>
UnaryExpressionCell<T>::UnaryExpressionCell(ExpressionKind kind, Expression<T> e, bool is_expanded)
    : ExpressionCell<T>{kind, is_expanded}, e_{e} {}
template <class T>
UnaryExpressionCell<T>::UnaryExpressionCell(ExpressionKind kind, Expression<T> e, bool is_polynomial, bool is_expanded)
    : ExpressionCell<T>{kind, is_polynomial, is_expanded}, e_{e} {}

/**
 * BinaryExpressionCell
 */
template <class T>
BinaryExpressionCell<T>::BinaryExpressionCell(ExpressionKind kind, Expression<T> e1, Expression<T> e2, bool is_expanded)
    : ExpressionCell<T>{kind, is_expanded}, e1_{e1}, e2_{e2} {}
template <class T>
void BinaryExpressionCell<T>::hash(DelegatingHasher& hasher) const {
  e1_.hash(hasher);
  e2_.hash(hasher);
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
  return do_evaluate(e1_.evaluate(env), e2_.evaluate(env));
}
template <class T>
void BinaryExpressionCell<T>::compute_variables(std::optional<Variables>& variables) const {
  SMATS_ASSERT(!variables.has_value(), "variables_ must not have a value yet");
  variables = e1_.variables() + e2_.variables();
}
template <class T>
void BinaryExpressionCell<T>::compute_is_polynomial(std::optional<bool>& is_polynomial) const {
  SMATS_ASSERT(!is_polynomial.has_value(), "is_polynomial_ must not have a value yet");
  is_polynomial = e1_.is_polynomial() && e2_.is_polynomial();
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
void ExpressionConstant<T>::compute_variables(std::optional<Variables>& variables) const {
  SMATS_ASSERT(!variables.has_value(), "variables_ must not have a value yet");
  variables = Variables{};
}
template <class T>
void ExpressionConstant<T>::compute_is_polynomial(std::optional<bool>& is_polynomial) const {
  SMATS_ASSERT(!is_polynomial.has_value(), "is_polynomial_ must not have a value yet");
  is_polynomial = true;
}
template <class T>
bool ExpressionConstant<T>::equal_to(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionConstant<T>::kind(), "Expressions must have the same kind");
  const ExpressionConstant<T>& constant_e = o.template to<::smats::ExpressionConstant>();
  return value_ == constant_e.value_;
}
template <class T>
bool ExpressionConstant<T>::less(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionConstant<T>::kind(), "Expressions must have the same kind");
  const ExpressionConstant<T>& constant_e = o.template to<::smats::ExpressionConstant>();
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
T& ExpressionConstant<T>::m_value() {
  ExpressionCell<T>::invalidate_cache();
  return const_cast<T&>(value_);
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
void ExpressionVar<T>::compute_variables(std::optional<Variables>& variables) const {
  SMATS_ASSERT(!variables.has_value(), "variables_ must not have a value yet");
  variables = Variables{var_};
}
template <class T>
void ExpressionVar<T>::compute_is_polynomial(std::optional<bool>& is_polynomial) const {
  SMATS_ASSERT(!is_polynomial.has_value(), "is_polynomial_ must not have a value yet");
  is_polynomial = true;
}
template <class T>
bool ExpressionVar<T>::equal_to(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionVar<T>::kind(), "Expressions must have the same kind");
  const ExpressionVar<T>& var_e = e.template to<::smats::ExpressionVar>();
  return var_.equal_to(var_e.var_);
}
template <class T>
bool ExpressionVar<T>::less(const ExpressionCell<T>& e) const {
  SMATS_ASSERT(e.kind() == ExpressionVar<T>::kind(), "Expressions must have the same kind");
  const ExpressionVar<T>& var_e = e.template to<::smats::ExpressionVar>();
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
void ExpressionNaN<T>::compute_variables(std::optional<Variables>& variables) const {
  SMATS_ASSERT(!variables.has_value(), "variables_ must not have a value yet");
  variables = Variables{};
}
template <class T>
void ExpressionNaN<T>::compute_is_polynomial(std::optional<bool>& is_polynomial) const {
  SMATS_ASSERT(!is_polynomial.has_value(), "is_polynomial_ must not have a value yet");
  is_polynomial = true;
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
    : ExpressionCell<T>{ExpressionKind::Add, false},
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
void ExpressionAdd<T>::compute_variables(std::optional<Variables>& variables) const {
  SMATS_ASSERT(!variables.has_value(), "variables_ must not have a value yet");
  variables = Variables{};
  for (const auto& [expr, _] : expr_to_coeff_map_) variables.value() += expr.variables();
}
template <class T>
void ExpressionAdd<T>::compute_is_polynomial(std::optional<bool>& is_polynomial) const {
  SMATS_ASSERT(!is_polynomial.has_value(), "is_polynomial_ must not have a value yet");
  is_polynomial = std::all_of(expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(),
                              [](const std::pair<Expression<T>, T>& el) { return el.first.is_polynomial(); });
}
template <class T>
bool ExpressionAdd<T>::equal_to(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionAdd<T>::kind(), "Expressions must have the same kind");
  const ExpressionAdd<T>& add_e = o.template to<::smats::ExpressionAdd>();
  if (constant_ != add_e.constant_ || expr_to_coeff_map_.size() != add_e.expr_to_coeff_map_.size()) return false;
  return std::equal(
      expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(), add_e.expr_to_coeff_map_.begin(),
      [](const std::pair<const Expression<T>, const T&> lhs, const std::pair<const Expression<T>, const T&> rhs) {
        return lhs.first.equal_to(rhs.first) && lhs.second == rhs.second;
      });
}
template <class T>
bool ExpressionAdd<T>::less(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionAdd<T>::kind(), "Expressions must have the same kind");
  const ExpressionAdd<T>& add_e = o.template to<::smats::ExpressionAdd>();
  if (constant_ < add_e.constant_) return true;
  if (add_e.constant_ < constant_) return false;

  return std::lexicographical_compare(
      expr_to_coeff_map_.cbegin(), expr_to_coeff_map_.cend(), add_e.expr_to_coeff_map_.cbegin(),
      add_e.expr_to_coeff_map_.cend(),
      [](const std::pair<const Expression<T>, T>& lhs, const std::pair<const Expression<T>, T>& rhs) {
        if (lhs.first.less(rhs.first)) return true;
        if (lhs.first.less(rhs.first)) return false;
        return lhs.second < rhs.second;
      });
}
template <class T>
T ExpressionAdd<T>::evaluate(const Environment<T>& env) const {
  return std::accumulate(expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(), constant_,
                         [&env](T acc, const auto& p) { return acc + p.second * p.first.evaluate(env); });
}
template <class T>
Expression<T> ExpressionAdd<T>::expand() const {
  Expression<T> result{constant_};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += Expression{expr * coeff}.expand();
  return result;
}
template <class T>
Expression<T> ExpressionAdd<T>::evaluate_partial(const Environment<T>& env) const {
  Expression<T> result{constant_};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += Expression{expr}.evaluate_partial(env) * coeff;
  return result;
}
template <class T>
Expression<T> ExpressionAdd<T>::substitute(const Substitution<T>& s) const {
  Expression<T> result{constant_};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += Expression{expr}.substitute(s) * coeff;
  return result;
}
template <class T>
Expression<T> ExpressionAdd<T>::differentiate(const Variable& x) const {
  Expression<T> result{0};
  for (const auto& [expr, coeff] : expr_to_coeff_map_) result += Expression{expr}.differentiate(x) * coeff;
  return result;
}
template <class T>
Variables ExpressionAdd<T>::extract_variables([[maybe_unused]] const ExpressionMap& expr_to_coeff_map) {
  SMATS_NOT_IMPLEMENTED();
}
template <class T>
T& ExpressionAdd<T>::m_constant() {
  ExpressionCell<T>::invalidate_cache();
  return const_cast<T&>(constant_);
}
template <class T>
ExpressionAdd<T>::ExpressionMap& ExpressionAdd<T>::m_expr_to_coeff_map() {
  ExpressionCell<T>::invalidate_cache();
  return const_cast<ExpressionMap&>(expr_to_coeff_map_);
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

/*
 * ExpressionMul
 */
template <class T>
ExpressionMul<T>::ExpressionMul(ExpressionCell<T>::Private, T constant, ExpressionMap base_to_exponent_map)
    : ExpressionCell<T>{ExpressionKind::Mul, false},
      constant_{std::move(constant)},
      base_to_exponent_map_{std::move(base_to_exponent_map)} {}
template <class T>
void ExpressionMul<T>::hash(smats::DelegatingHasher& hasher) const {
  hash_append(hasher, constant_);
  for (const auto& [base, exp] : base_to_exponent_map_) {
    hash_append(hasher, base);
    hash_append(hasher, exp);
  }
}
template <class T>
void ExpressionMul<T>::compute_variables(std::optional<Variables>& variables) const {
  SMATS_ASSERT(!variables.has_value(), "variables_ must not have a value yet");
  variables = Variables{};
  for (const auto& [expr, _] : base_to_exponent_map_) variables.value() += expr.variables();
}
template <class T>
void ExpressionMul<T>::compute_is_polynomial(std::optional<bool>& is_polynomial) const {
  SMATS_ASSERT(!is_polynomial.has_value(), "is_polynomial_ must not have a value yet");
  is_polynomial = std::all_of(base_to_exponent_map_.begin(), base_to_exponent_map_.end(),
                              [](const std::pair<const Expression<T>, Expression<T>>& p) {
                                // For each base^exponent, it has to satisfy the following
                                // conditions:
                                //     - base is polynomial-convertible.
                                //     - exponent is a non-negative integer.
                                const Expression<T>& base{p.first};
                                const Expression<T>& exponent{p.second};
                                if (!base.is_polynomial() || !exponent.is_constant()) {
                                  return false;
                                }
                                const T& e{exponent.constant()};
                                return e >= 0 && is_integer(e);
                              });
}
template <class T>
bool ExpressionMul<T>::equal_to(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionMul<T>::kind(), "Expressions must have the same kind");
  const ExpressionMul<T>& mul_e = o.template to<::smats::ExpressionMul>();
  if (constant_ != mul_e.constant_ || base_to_exponent_map_.size() != mul_e.base_to_exponent_map_.size()) return false;
  return std::equal(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), mul_e.base_to_exponent_map_.begin(),
                    [](const std::pair<const Expression<T>, const Expression<T>>& lhs,
                       const std::pair<const Expression<T>, const Expression<T>>& rhs) {
                      return lhs.first.equal_to(rhs.first) && lhs.second.equal_to(rhs.second);
                    });
}
template <class T>
bool ExpressionMul<T>::less(const ExpressionCell<T>& o) const {
  SMATS_ASSERT(o.kind() == ExpressionMul<T>::kind(), "Expressions must have the same kind");
  const ExpressionMul<T>& mul_e = o.template to<::smats::ExpressionMul>();
  if (constant_ < mul_e.constant_) return true;
  if (mul_e.constant_ < constant_) return false;

  return std::lexicographical_compare(base_to_exponent_map_.cbegin(), base_to_exponent_map_.cend(),
                                      mul_e.base_to_exponent_map_.cbegin(), mul_e.base_to_exponent_map_.cend(),
                                      [](const std::pair<const Expression<T>, const Expression<T>>& lhs,
                                         const std::pair<const Expression<T>, const Expression<T>>& rhs) {
                                        if (lhs.first.less(rhs.first)) return true;
                                        if (lhs.first.less(rhs.first)) return false;
                                        return lhs.second.less(rhs.second);
                                      });
}
template <class T>
T ExpressionMul<T>::evaluate(const Environment<T>& env) const {
  return std::accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), constant_,
                         [&env](const T& init, const std::pair<const Expression<T>, Expression<T>>& p) {
                           const T exponent{p.second.evaluate(env)};
                           if (exponent == 1) return static_cast<T>(init * p.first.evaluate(env));
                           if (exponent == 0) return init;
                           return static_cast<T>(std::pow(p.first.evaluate(env), exponent));
                         });
}
template <class T>
Expression<T> ExpressionMul<T>::expand() const {
  //   (c * ∏ᵢ pow(bᵢ, eᵢ)).Expand()
  // = c * expand_multiplication(∏ expand_pow(bᵢ.Expand(), eᵢ.Expand()))
  return std::accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), Expression{constant_},
                         [](const Expression<T>& init, const std::pair<const Expression<T>, Expression<T>>& p) {
                           const Expression<T>& b_i{p.first};
                           const Expression<T>& e_i{p.second};
                           return expand_multiplication(init, expand_pow(b_i.is_expanded() ? b_i : b_i.expand(),
                                                                         e_i.is_expanded() ? e_i : e_i.expand()));
                         });
}
template <class T>
Expression<T> ExpressionMul<T>::evaluate_partial(const Environment<T>& env) const {
  return std::accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), Expression{constant_},
                         [&env](const Expression<T>& init, const std::pair<const Expression<T>, Expression<T>>& p) {
                           return init * (p.first.evaluate_partial(env) ^ p.second.evaluate_partial(env));
                         });
}
template <class T>
Expression<T> ExpressionMul<T>::substitute(const Substitution<T>& s) const {
  return std::accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), Expression{constant_},
                         [&s](const Expression<T>& init, const std::pair<const Expression<T>, Expression<T>>& p) {
                           return init * (p.first.substitute(s) ^ p.second.substitute(s));
                         });
}
template <class T>
Expression<T> ExpressionMul<T>::differentiate(const Variable& x) const {
  SMATS_NOT_IMPLEMENTED();
}
template <class T>
Variables ExpressionMul<T>::extract_variables([[maybe_unused]] const ExpressionMap& base_to_exponent_map) {
  SMATS_NOT_IMPLEMENTED();
}
template <class T>
T& ExpressionMul<T>::m_constant() {
  ExpressionCell<T>::invalidate_cache();
  return const_cast<T&>(constant_);
}
template <class T>
ExpressionMul<T>::ExpressionMap& ExpressionMul<T>::m_base_to_exponent_map() {
  ExpressionCell<T>::invalidate_cache();
  return const_cast<ExpressionMap&>(base_to_exponent_map_);
}
template <class T>
std::ostream& ExpressionMul<T>::display(std::ostream& os) const {
  SMATS_ASSERT(!base_to_exponent_map_.empty(), "ExpressionMul must have at least one term");
  bool print_mul = false;
  os << "(";
  if (constant_ != 1) {
    os << constant_;
    print_mul = true;
  }
  for (const auto& p : base_to_exponent_map_) {
    display_term(os, print_mul, p.first, p.second);
    print_mul = true;
  }
  return os << ")";
}
template <class T>
std::ostream& ExpressionMul<T>::display_term(std::ostream& os, const bool print_mul, const Expression<T>& base,
                                             const Expression<T>& exponent) const {
  if (print_mul) os << " * ";
  return (exponent.is_constant(1) ? os << base : os << "pow(" << base << ", " << exponent << ")");
}

/**
 * ExpressionPow
 */
template <class T>
ExpressionPow<T>::ExpressionPow(ExpressionCell<T>::Private, const Expression<T>& base, const Expression<T>& exponent)
    : BinaryExpressionCell<T>{ExpressionKind::Pow, base, exponent, base.is_leaf() && exponent.is_leaf()} {}
template <class T>
void ExpressionPow<T>::compute_is_polynomial(std::optional<bool>& is_polynomial) const {
  SMATS_ASSERT(!is_polynomial.has_value(), "is_polynomial_ must not have a value yet");
  // base ^ exponent is polynomial-convertible if the followings hold:
  //    - base is polynomial-convertible.
  //    - exponent is a non-negative integer.
  if (!(BinaryExpressionCell<T>::lhs().is_polynomial() && BinaryExpressionCell<T>::rhs().is_constant())) {
    is_polynomial = false;
    return;
  }
  const T& e{BinaryExpressionCell<T>::rhs().constant()};
  is_polynomial = e >= 0 && is_integer(e);
}

template <class T>
Expression<T> ExpressionPow<T>::expand() const {
  const Expression<T>& arg1 = BinaryExpressionCell<T>::lhs();
  const Expression<T>& arg2 = BinaryExpressionCell<T>::rhs();
  const Expression<T> arg1_expanded{arg1.expand()};
  const Expression<T> arg2_expanded{arg2.expand()};
  return expand_pow(arg1_expanded, arg2_expanded);
}
template <class T>
Expression<T> ExpressionPow<T>::evaluate_partial(const Environment<T>& env) const {
  return BinaryExpressionCell<T>::lhs().evaluate_partial(env) ^ BinaryExpressionCell<T>::rhs().evaluate_partial(env);
}
template <class T>
Expression<T> ExpressionPow<T>::substitute(const Substitution<T>& s) const {
  const Expression<T>& arg1 = BinaryExpressionCell<T>::lhs();
  const Expression<T>& arg2 = BinaryExpressionCell<T>::rhs();
  const Expression<T> arg1_subst{arg1.substitute(s)};
  const Expression<T> arg2_subst{arg2.substitute(s)};
  return !arg1.equal_to(arg1_subst) || !arg2.equal_to(arg2_subst) ? (arg1_subst ^ arg2_subst)
                                                                  : ExpressionCell<T>::to_expression();
}
template <class T>
Expression<T> ExpressionPow<T>::differentiate(const Variable& x) const {
  SMATS_NOT_IMPLEMENTED();
}
template <class T>
std::ostream& ExpressionPow<T>::display(std::ostream& os) const {
  return os << "(" << BinaryExpressionCell<T>::lhs() << " ^ " << BinaryExpressionCell<T>::rhs() << ")";
}
template <class T>
void ExpressionPow<T>::check_domain(const T& v1, const T& v2) {
  if (std::isfinite(v1) && (v1 < 0.0) && std::isfinite(v2) && !is_integer(v2)) {
    SMATS_RUNTIME_ERROR_FMT(
        "pow({}, {}) : numerical argument out of domain. {} is finite negative and {} is finite non-integer", v1, v2,
        v1, v2);
  }
}
template <>
void ExpressionPow<int>::check_domain(const int& v1, const int& v2) {}
template <>
void ExpressionPow<long>::check_domain(const long& v1, const long& v2) {}
template <class T>
T ExpressionPow<T>::do_evaluate(const T& num, const T& den) const {
  check_domain(num, den);
  return static_cast<T>(std::pow(num, den));
}

/**
 * ExpressionDiv
 */
template <class T>
ExpressionDiv<T>::ExpressionDiv(ExpressionCell<T>::Private, const Expression<T>& e1, const Expression<T>& e2)
    : BinaryExpressionCell<T>{ExpressionKind::Div, e1, e2, false} {}
template <class T>
Expression<T> ExpressionDiv<T>::expand() const {
  const Expression<T>& first = BinaryExpressionCell<T>::lhs();
  const Expression<T>& second = BinaryExpressionCell<T>::rhs();
  const Expression<T> e1{first.is_expanded() ? first : first.expand()};
  const Expression<T> e2{second.is_expanded() ? second : second.expand()};
  SMATS_NOT_IMPLEMENTED();
  //  return e2.is_constant() ? expand_division(e1, e2) : e1 / e2;
  //    return DivExpandVisitor{}.Simplify(e1, get_constant_value(e2));
}
template <class T>
Expression<T> ExpressionDiv<T>::evaluate_partial(const Environment<T>& env) const {
  return BinaryExpressionCell<T>::lhs().evaluate_partial(env) / BinaryExpressionCell<T>::rhs().evaluate_partial(env);
}
template <class T>
T ExpressionDiv<T>::do_evaluate(const T& num, const T& den) const {
  if (num == 0 && den == 0) SMATS_RUNTIME_ERROR("Indeterminate form: 0 / 0");
  if (den == 0) SMATS_RUNTIME_ERROR_FMT("Division by zero: {} / 0", num);
  return num / den;
}
template <class T>
Expression<T> ExpressionDiv<T>::substitute(const Substitution<T>& s) const {
  return BinaryExpressionCell<T>::lhs().substitute(s) / BinaryExpressionCell<T>::rhs().substitute(s);
}
template <class T>
Expression<T> ExpressionDiv<T>::differentiate(const Variable& x) const {
  // ∂/∂x (f / g) = (∂/∂x f * g - f * ∂/∂x g) / g^2
  const Expression<T>& f{BinaryExpressionCell<T>::lhs()};
  const Expression<T>& g{BinaryExpressionCell<T>::rhs()};
  return (f.differentiate(x) * g - f * g.differentiate(x)) / (g ^ 2);
}
template <class T>
void ExpressionDiv<T>::check_domain(const T& v1, const T& v2) {
  SMATS_NOT_IMPLEMENTED();
}
template <class T>
std::ostream& ExpressionDiv<T>::display(std::ostream& os) const {
  return os << "(" << BinaryExpressionCell<T>::lhs() << " / " << BinaryExpressionCell<T>::rhs() << ")";
}

EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionCell);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(UnaryExpressionCell);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(BinaryExpressionCell);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionConstant);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionVar);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionNaN);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionAdd);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionMul);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionPow);
EXPLICIT_TEMPLATE_INSTANTIATION_NUMERIC(ExpressionDiv);

}  // namespace smats
