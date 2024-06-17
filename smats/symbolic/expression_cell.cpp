#include "smats/symbolic/expression_cell.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "smats/util/exception.h"

namespace smats {

namespace {

template <class T>
bool is_integer(const T& v) {
  return std::numeric_limits<int>::lowest() <= v && v <= std::numeric_limits<int>::max();
}
template <>
bool is_integer(const mpq_class& v) {
  return v.get_den() == 1 && std::numeric_limits<int>::lowest() <= v && v <= std::numeric_limits<int>::max();
}

template <class T>
bool is_positive_integer(const mpq_class& v) {
  return (v > 0) && is_integer<T>(v);
}
template <class T>
bool is_non_negative_integer(const mpq_class& v) {
  return (v >= 0) && is_integer<T>(v);
}

// Returns true iff `e` does not wrap another sub-Expression, i.e., it's not a
// UnaryExpressionCell, BinaryExpressionCell, etc.
//
// Pedantially, ExpressionNaN should return `true` here since it isn't wrapping
// anything, but given the practical uses of this function it's easier to just
// return `false` for NaNs.
bool is_leaf_expression(const Expression& e) { return is_constant(e) || is_variable(e); }

// Determines if the summation represented by term_to_coeff_map is
// polynomial-convertible or not. This function is used in the
// constructor of ExpressionAdd.
template <class T>
bool determine_polynomial(const std::map<Expression<T>, T>& term_to_coeff_map) {
  return all_of(term_to_coeff_map.begin(), term_to_coeff_map.end(),
                [](const std::pair<const Expression<T>, T>& p) { return p.first.is_polynomial(); });
}

#if 0
// Determines if the product represented by term_to_coeff_map is
// polynomial-convertible or not. This function is used in the
// constructor of ExpressionMul.
bool determine_polynomial(const std::map<Expression, Expression>& base_to_exponent_map) {
  return all_of(base_to_exponent_map.begin(), base_to_exponent_map.end(),
                [](const std::pair<const Expression, Expression>& p) {
                  // For each base^exponent, it has to satisfy the following
                  // conditions:
                  //     - base is polynomial-convertible.
                  //     - exponent is a non-negative integer.
                  const Expression& base{p.first};
                  const Expression& exponent{p.second};
                  if (!base.is_polynomial() || !is_constant(exponent)) {
                    return false;
                  }
                  const mpq_class e{get_constant_value(exponent)};
                  return is_non_negative_integer(e);
                });
}

// Determines if pow(base, exponent) is polynomial-convertible or not. This
// function is used in constructor of ExpressionPow.
bool determine_polynomial(const Expression& base, const Expression& exponent) {
  // base ^ exponent is polynomial-convertible if the following hold:
  //    - base is polynomial-convertible.
  //    - exponent is a non-negative integer.
  if (!(base.is_polynomial() && is_constant(exponent))) {
    return false;
  }
  const mpq_class e{get_constant_value(exponent)};
  return is_non_negative_integer(e);
}

Expression ExpandMultiplication(const Expression& e1, const Expression& e2, const Expression& e3);

// Helper function expanding (e1 * e2). It assumes that both of e1 and e2 are
// already expanded.
Expression ExpandMultiplication(const Expression& e1, const Expression& e2) {
  // Precondition: e1 and e2 are already expanded.
  SMATS_ASSERT(e1.equal_to(e1.Expand()), "e1 must have been already expanded.");
  SMATS_ASSERT(e2.equal_to(e2.Expand()), "e2 must have been already expanded.");

  if (is_addition(e1)) {
    //   (c0 + c1 * e_{1,1} + ... + c_n * e_{1, n}) * e2
    // = c0 * e2 + c1 * e_{1,1} * e2 + ... + c_n * e_{1,n} * e2
    const mpq_class c0{get_constant_in_addition(e1)};
    const std::map<Expression, mpq_class>& m1{get_expr_to_coeff_map_in_addition(e1)};
    ExpressionAddFactory fac;
    fac.AddExpression(ExpandMultiplication(c0, e2));
    for (const std::pair<const Expression, mpq_class>& p : m1) {
      fac.AddExpression(ExpandMultiplication(p.second, p.first, e2));
    }
    return std::move(fac).GetExpression();
  }
  if (is_addition(e2)) {
    //   e1 * (c0 + c1 * e_{2,1} + ... + c_n * e_{2, n})
    // = e1 * c0 + e1 * c1 * e_{2,1} + ... + e1 * c_n * e_{2,n}
    const mpq_class c0{get_constant_in_addition(e2)};
    const std::map<Expression, mpq_class>& m1{get_expr_to_coeff_map_in_addition(e2)};
    ExpressionAddFactory fac;
    fac.AddExpression(ExpandMultiplication(e1, c0));
    for (const std::pair<const Expression, mpq_class>& p : m1) {
      fac.AddExpression(ExpandMultiplication(e1, p.second, p.first));
    }
    return std::move(fac).GetExpression();
  }
  if (is_division(e1)) {
    const Expression& e1_1{get_first_argument(e1)};
    const Expression& e1_2{get_second_argument(e1)};
    if (is_division(e2)) {
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
  if (is_division(e2)) {
    //    (e1 * (e2_1 / e2_2)).Expand()
    // => (e1 * e2_1).Expand() / e2_2.
    //
    // Note that e1, e2_1, and e2_2 are already expanded by the precondition.
    const Expression& e2_1{get_first_argument(e2)};
    const Expression& e2_2{get_second_argument(e2)};
    return ExpandMultiplication(e1, e2_1) / e2_2;
  }
  return e1 * e2;
}

Expression ExpandMultiplication(const Expression& e1, const Expression& e2, const Expression& e3) {
  return ExpandMultiplication(ExpandMultiplication(e1, e2), e3);
}

// Helper function expanding pow(base, n). It assumes that base is expanded.
Expression ExpandPow(const Expression& base, const long n) {
  // Precondition: base is already expanded.
  SMATS_ASSERT(base.equal_to(base.Expand()), "Base must have been already expanded.");
  SMATS_ASSERT(n >= 1, "Exponent must be a positive integer.");
  if (n == 1) {
    return base;
  }
  const Expression pow_half{ExpandPow(base, n / 2)};
  if (n % 2 == 1) {
    // pow(base, n) = base * pow(base, n / 2) * pow(base, n / 2)
    return ExpandMultiplication(base, pow_half, pow_half);
  }
  // pow(base, n) = pow(base, n / 2) * pow(base, n / 2)
  return ExpandMultiplication(pow_half, pow_half);
}

/**
 * Helper function expanding pow(base, exponent).
 * @pre base is already expanded.
 * @pre exponent is already expanded.
 * @param base base of the exponentiation
 * @param exponent exponent of the exponentiation
 * @return expanded expression of pow(base, exponent)
 */
Expression ExpandPow(const Expression& base, const Expression& exponent) {
  // Precondition: base and exponent are already expanded.
  SMATS_ASSERT(base.equal_to(base.Expand()), "Base must have been already expanded.");
  SMATS_ASSERT(exponent.equal_to(exponent.Expand()), "Exponent must have been already expanded");
  if (is_multiplication(base)) {
    //   pow(c * ∏ᵢ pow(e₁ᵢ, e₂ᵢ), exponent)
    // = pow(c, exponent) * ∏ᵢ pow(e₁ᵢ, e₂ᵢ * exponent)
    const mpq_class c{get_constant_in_multiplication(base)};
    auto map = get_base_to_exponent_map_in_multiplication(base);
    for (std::pair<const Expression, Expression>& p : map) {
      p.second = p.second * exponent;
    }
    return pow(c, exponent) * ExpressionMulFactory{1.0, map}.GetExpression();
  }

  // Expand if
  //     1) base is an addition expression and
  //     2) exponent is a positive integer.
  if (!is_addition(base) || !is_constant(exponent)) {
    return pow(base, exponent);
  }
  const mpq_class e{get_constant_value(exponent)};
  if (e <= 0 || !is_integer(e)) {
    return pow(base, exponent);
  }
  return ExpandPow(base, e.get_num().get_si());
}
#endif
}  // namespace

ExpressionCell::ExpressionCell(const ExpressionKind k, const bool is_poly, const bool is_expanded)
    : kind_{k}, is_polynomial_{is_poly}, is_expanded_{is_expanded} {}

UnaryExpressionCell::UnaryExpressionCell(const ExpressionKind k, Expression e, const bool is_poly,
                                         const bool is_expanded)
    : ExpressionCell{k, is_poly, is_expanded}, e_{std::move(e)} {}

void UnaryExpressionCell::hash(DelegatingHasher& hasher) const { hash_append(hasher, e_); }

Variables UnaryExpressionCell::variables() const { return e_.variables(); }

bool UnaryExpressionCell::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const auto& unary_e = static_cast<const UnaryExpressionCell&>(e);
  return e_.equal_to(unary_e.e_);
}

bool UnaryExpressionCell::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const auto& unary_e = static_cast<const UnaryExpressionCell&>(e);
  return e_.less(unary_e.e_);
}

template <class T>
T UnaryExpressionCell::evaluate(const Environment<T>& env) const {
  const T v{e_.evaluate(env)};
  return do_evaluate(v);
}

BinaryExpressionCell::BinaryExpressionCell(const ExpressionKind k, Expression e1, Expression e2, const bool is_poly,
                                           const bool is_expanded)
    : ExpressionCell{k, is_poly, is_expanded}, e1_{std::move(e1)}, e2_{std::move(e2)} {}

void BinaryExpressionCell::hash(DelegatingHasher& hasher) const {
  hash_append(hasher, e1_);
  hash_append(hasher, e2_);
}

Variables BinaryExpressionCell::variables() const {
  Variables ret{e1_.variables()};
  ret.insert(e2_.variables());
  return ret;
}

bool BinaryExpressionCell::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const auto& binary_e = static_cast<const BinaryExpressionCell&>(e);
  return e1_.equal_to(binary_e.e1_) && e2_.equal_to(binary_e.e2_);
}

bool BinaryExpressionCell::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const auto& binary_e = static_cast<const BinaryExpressionCell&>(e);
  if (e1_.less(binary_e.e1_)) {
    return true;
  }
  if (binary_e.e1_.less(e1_)) {
    return false;
  }
  // e1_ equals to binary_e.e1_, compare e2_ and binary_e.e2_
  return e2_.less(binary_e.e2_);
}

template <class T>
T BinaryExpressionCell::evaluate(const Environment<T>& env) const {
  const T v1{e1_.evaluate(env)};
  const T v2{e2_.evaluate(env)};
  return do_evaluate(v1, v2);
}

ExpressionVar::ExpressionVar(Private, Variable v)
    : ExpressionCell{ExpressionKind::Var, true, true}, var_{std::move(v)} {
  // Boolean symbolic variable should not be used in constructing symbolic expressions.
  if (var_.type() == Variable::Type::BOOLEAN) {
    throw std::domain_error{"Boolean symbolic variable should not be used in constructing symbolic expressions."};
  }
}

void ExpressionVar::hash(DelegatingHasher& hasher) const { hash_append(hasher, var_); }

Variables ExpressionVar::variables() const { return {get_variable()}; }

bool ExpressionVar::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  return var_.equal_to(static_cast<const ExpressionVar&>(e).var_);
}

bool ExpressionVar::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  // Note the below is using the overloaded operator< between ExpressionVar
  // which is based on variable IDs.
  return var_.less(static_cast<const ExpressionVar&>(e).var_);
}

template <class T>
T ExpressionVar::evaluate(const Environment<T>& env) const {
  Environment<T>::const_iterator const it{env.find(var_)};
  if (it != env.cend()) return it->second;
  SMATS_RUNTIME_ERROR_FMT("The environment does not have an entry for the variable {}", var_);
}

Expression ExpressionVar::expand() const { return Expression{var_}; }

template <class T>
Expression ExpressionVar::evaluate_partial(const Environment<T>& env) const {
  const Environment<T>::const_iterator it{env.find(var_)};
  if (it != env.end()) {
    return it->second;
  }
  return Expression{var_};
}

Expression ExpressionVar::substitute(const Substitution& s) const {
  const Substitution::const_iterator it{s.find(var_)};
  if (it != s.end()) {
    return it->second;
  }
  return Expression{var_};
}

Expression ExpressionVar::differentiate(const Variable& x) const {
  if (x.equal_to(var_)) {
    return Expression::One();
  }
  return Expression::Zero();
}

std::ostream& ExpressionVar::display(std::ostream& os) const { return os << var_; }

ExpressionNaN::ExpressionNaN() : ExpressionCell{ExpressionKind::NaN, false, false} {}

void ExpressionNaN::hash(DelegatingHasher&) const {}

Variables ExpressionNaN::variables() const { return Variables{}; }

bool ExpressionNaN::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  return true;
}

bool ExpressionNaN::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  return false;
}

template <class T>
T ExpressionNaN::Evaluate(const Environment<T>&) const {
  SMATS_NAN_ERROR();
}
Expression ExpressionNaN::expand() const { SMATS_NAN_ERROR(); }
Expression ExpressionNaN::evaluate_partial(const Environment&) const { SMATS_NAN_ERROR(); }
Expression ExpressionNaN::substitute(const Substitution&) const { SMATS_NAN_ERROR(); }
Expression ExpressionNaN::differentiate(const Variable&) const { SMATS_NAN_ERROR(); }

std::ostream& ExpressionNaN::Display(std::ostream& os) const { return os << "NaN"; }

#if 0

ExpressionAdd::ExpressionAdd(const mpq_class constant, std::map<Expression, mpq_class> expr_to_coeff_map)
    : ExpressionCell{ExpressionKind::Add, determine_polynomial(expr_to_coeff_map), false},
      constant_(constant),
      expr_to_coeff_map_(std::move(expr_to_coeff_map)) {
  SMATS_ASSERT(!expr_to_coeff_map_.empty(), "The map must not be empty.");
}

void ExpressionAdd::hash(DelegatingHasher& hasher) const {
  hash_append(hasher, constant_);
  hash_append(hasher, expr_to_coeff_map_);
}

Variables ExpressionAdd::variables() const {
  Variables ret{};
  for (const auto& p : expr_to_coeff_map_) {
    ret.insert(p.first.variables());
  }
  return ret;
}

bool ExpressionAdd::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionAdd& add_e{static_cast<const ExpressionAdd&>(e)};
  // Compare constant.
  if (constant_ != add_e.constant_) {
    return false;
  }
  return equal(expr_to_coeff_map_.cbegin(), expr_to_coeff_map_.cend(), add_e.expr_to_coeff_map_.cbegin(),
               add_e.expr_to_coeff_map_.cend(),
               [](const std::pair<const Expression, mpq_class>& p1, const std::pair<const Expression, mpq_class>& p2) {
                 return p1.first.equal_to(p2.first) && p1.second == p2.second;
               });
}

bool ExpressionAdd::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionAdd& add_e{static_cast<const ExpressionAdd&>(e)};
  // Compare the constants.
  if (constant_ < add_e.constant_) {
    return true;
  }
  if (add_e.constant_ < constant_) {
    return false;
  }
  // Compare the two maps.
  return lexicographical_compare(
      expr_to_coeff_map_.cbegin(), expr_to_coeff_map_.cend(), add_e.expr_to_coeff_map_.cbegin(),
      add_e.expr_to_coeff_map_.cend(),
      [](const std::pair<const Expression, mpq_class>& p1, const std::pair<const Expression, mpq_class>& p2) {
        const Expression& term1{p1.first};
        const Expression& term2{p2.first};
        if (term1.less(term2)) {
          return true;
        }
        if (term2.less(term1)) {
          return false;
        }
        const mpq_class coeff1{p1.second};
        const mpq_class coeff2{p2.second};
        return coeff1 < coeff2;
      });
}

mpq_class ExpressionAdd::Evaluate(const Environment& env) const {
  return accumulate(expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(), constant_,
                    [&env](const mpq_class init, const std::pair<const Expression, mpq_class>& p) {
                      return init + p.first.Evaluate(env) * p.second;
                    });
}

Expression ExpressionAdd::Expand() const {
  //   (c0 + c1 * e_1 + ... + c_n * e_n).Expand()
  // =  c0 + c1 * e_1.Expand() + ... + c_n * e_n.Expand()
  ExpressionAddFactory fac{constant_, {}};
  for (const std::pair<const Expression, mpq_class>& p : expr_to_coeff_map_) {
    const Expression& e_i{p.first};
    const mpq_class c_i{p.second};
    fac.AddExpression(ExpandMultiplication(e_i.is_expanded() ? e_i : e_i.Expand(), c_i));
  }
  return std::move(fac).GetExpression();
}

Expression ExpressionAdd::EvaluatePartial(const Environment& env) const {
  return accumulate(expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(), Expression{constant_},
                    [&env](const Expression& init, const std::pair<const Expression, mpq_class>& p) {
                      return init + p.first.EvaluatePartial(env) * p.second;
                    });
}

Expression ExpressionAdd::Substitute(const Substitution& s) const {
  return accumulate(expr_to_coeff_map_.begin(), expr_to_coeff_map_.end(), Expression{constant_},
                    [&s](const Expression& init, const std::pair<const Expression, mpq_class>& p) {
                      return init + p.first.Substitute(s) * p.second;
                    });
}

Expression ExpressionAdd::Differentiate(const Variable& x) const {
  //   ∂/∂x (c_0 + c_1 * f_1 + ... + c_n * f_n)
  // = (∂/∂x c_0) + (∂/∂x c_1 * f_1) + ... + (∂/∂x c_n * f_n)
  // =  0.0       + c_1 * (∂/∂x f_1) + ... + c_n * (∂/∂x f_n)
  ExpressionAddFactory fac;
  for (const std::pair<const Expression, mpq_class>& p : expr_to_coeff_map_) {
    fac.AddExpression(p.second * p.first.Differentiate(x));
  }
  return std::move(fac).GetExpression();
}

std::ostream& ExpressionAdd::Display(std::ostream& os) const {
  SMATS_ASSERT(!expr_to_coeff_map_.empty(), "The map must not be empty.");
  bool print_plus{false};
  os << "(";
  if (constant_ != 0.0) {
    os << constant_;
    print_plus = true;
  }
  for (const auto& p : expr_to_coeff_map_) {
    DisplayTerm(os, print_plus, p.second, p.first);
    print_plus = true;
  }
  os << ")";
  return os;
}

std::ostream& ExpressionAdd::DisplayTerm(std::ostream& os, const bool print_plus, const mpq_class coeff,
                                         const Expression& term) const {
  SMATS_ASSERT(coeff != 0.0, "The coefficient must not be zero.");
  if (coeff > 0.0) {
    if (print_plus) {
      os << " + ";
    }
    // Do not print "1 * t"
    if (coeff != 1.0) {
      os << coeff << " * ";
    }
  } else {
    // Instead of printing "+ (- E)", just print "- E".
    os << " - ";
    if (coeff != -1.0) {
      os << (-coeff) << " * ";
    }
  }
  os << term;
  return os;
}

ExpressionAddFactory::ExpressionAddFactory(const mpq_class constant, std::map<Expression, mpq_class> expr_to_coeff_map)
    : is_expanded_(false), constant_{constant}, expr_to_coeff_map_{std::move(expr_to_coeff_map)} {
  // Note that we set is_expanded_ to false to be conservative. We could imagine
  // inspecting the expr_to_coeff_map to compute a more precise value, but it's
  // not clear that doing so would improve our overall performance.
}

ExpressionAddFactory::ExpressionAddFactory(const ExpressionAdd& add)
    : ExpressionAddFactory{add.get_constant(), add.get_expr_to_coeff_map()} {
  is_expanded_ = add.is_expanded();
}

void ExpressionAddFactory::AddExpression(const Expression& e) {
  if (is_constant(e)) {
    const mpq_class v{get_constant_value(e)};
    return AddConstant(v);
  }
  if (is_addition(e)) {
    // Flattening
    return Add(to_addition(e));
  }
  if (is_multiplication(e)) {
    const mpq_class constant{get_constant_in_multiplication(e)};
    SMATS_ASSERT(constant != 0.0, "The constant must not be zero.");
    if (constant != 1.0) {
      // Instead of adding (1.0 * (constant * b1^t1 ... bn^tn)),
      // add (constant, 1.0 * b1^t1 ... bn^tn).
      return AddTerm(constant,
                     ExpressionMulFactory(1.0, get_base_to_exponent_map_in_multiplication(e)).GetExpression());
    }
  }
  return AddTerm(1.0, e);
}

void ExpressionAddFactory::Add(const ExpressionAdd& add) {
  AddConstant(add.get_constant());
  AddMap(add.get_expr_to_coeff_map());
}

ExpressionAddFactory& ExpressionAddFactory::operator=(const ExpressionAdd& add) {
  is_expanded_ = add.is_expanded();
  constant_ = add.get_constant();
  expr_to_coeff_map_ = add.get_expr_to_coeff_map();
  return *this;
}

ExpressionAddFactory&& ExpressionAddFactory::Negate() && {
  constant_ = -constant_;
  for (auto& p : expr_to_coeff_map_) {
    p.second = -p.second;
  }
  return std::move(*this);
}

Expression ExpressionAddFactory::GetExpression() && {
  if (expr_to_coeff_map_.empty()) {
    return Expression{constant_};
  }
  if (constant_ == 0.0 && expr_to_coeff_map_.size() == 1U) {
    // 0.0 + c1 * t1 -> c1 * t1
    const auto it(expr_to_coeff_map_.cbegin());
    return it->first * it->second;
  }
  auto result = make_unique<ExpressionAdd>(constant_, std::move(expr_to_coeff_map_));
  if (is_expanded_) {
    result->set_expanded();
  }
  return Expression{std::move(result)};
}

void ExpressionAddFactory::AddConstant(const mpq_class constant) { constant_ += constant; }

void ExpressionAddFactory::AddTerm(const mpq_class coeff, const Expression& term) {
  SMATS_ASSERT(!is_constant(term), "The term must not be a constant.");
  SMATS_ASSERT(coeff != 0.0, "The coefficient must not be zero.");

  const auto it(expr_to_coeff_map_.find(term));
  if (it != expr_to_coeff_map_.end()) {
    // Case1: term is already in the map
    mpq_class& this_coeff{it->second};
    this_coeff += coeff;
    if (this_coeff == 0.0) {
      // If the coefficient becomes zero, remove the entry.
      // TODO(soonho-tri): The following operation is not sound since it cancels
      // `term` which might contain 0/0 problems.
      expr_to_coeff_map_.erase(it);
      // Note that we leave is_expanded_ unchanged here. We could imagine
      // inspecting the new expr_to_coeff_map_ to compute a more precise value,
      // but it's not clear that doing so would improve our overall performance.
    }
  } else {
    // Case2: term is not found in expr_to_coeff_map_.
    // Add the entry (term, coeff).
    expr_to_coeff_map_.emplace(term, coeff);
    // If the addend is not a leaf cell (i.e., if it's a cell type that nests
    // another Expression inside of itself), then conservatively we can no
    // longer be sure that our ExpressionAdd remains in expanded form. For
    // example calling `AddTerm(5, 2 + 3 * y)` produces a non-expanded result
    // `5 * (2 + 3 * y)`, i.e., `{2 + 3 * y} => 5` in the expr_to_coeff_map_.
    if (!is_variable(term)) {
      is_expanded_ = false;
    }
  }
}

void ExpressionAddFactory::AddMap(const std::map<Expression, mpq_class>& expr_to_coeff_map) {
  for (const auto& p : expr_to_coeff_map) {
    AddTerm(p.second, p.first);
  }
}

ExpressionMul::ExpressionMul(const mpq_class constant, std::map<Expression, Expression> base_to_exponent_map)
    : ExpressionCell{ExpressionKind::Mul, determine_polynomial(base_to_exponent_map), false},
      constant_(constant),
      base_to_exponent_map_(std::move(base_to_exponent_map)) {
  SMATS_ASSERT(!base_to_exponent_map_.empty(), "The map must not be empty.");
}

void ExpressionMul::hash(DelegatingHasher& hasher) const {
  using smats::hash_append;
  hash_append(hasher, constant_);
  hash_append(hasher, base_to_exponent_map_);
}

Variables ExpressionMul::variables() const {
  Variables ret{};
  for (const auto& p : base_to_exponent_map_) {
    ret.insert(p.first.variables());
    ret.insert(p.second.variables());
  }
  return ret;
}

bool ExpressionMul::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionMul& mul_e{static_cast<const ExpressionMul&>(e)};
  // Compare constant.
  if (constant_ != mul_e.constant_) {
    return false;
  }
  // Check each (term, coeff) pairs in two maps.
  return equal(
      base_to_exponent_map_.cbegin(), base_to_exponent_map_.cend(), mul_e.base_to_exponent_map_.cbegin(),
      mul_e.base_to_exponent_map_.cend(),
      [](const std::pair<const Expression, Expression>& p1, const std::pair<const Expression, Expression>& p2) {
        return p1.first.equal_to(p2.first) && p1.second.equal_to(p2.second);
      });
}

bool ExpressionMul::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionMul& mul_e{static_cast<const ExpressionMul&>(e)};
  // Compare the constants.
  if (constant_ < mul_e.constant_) {
    return true;
  }
  if (mul_e.constant_ < constant_) {
    return false;
  }
  // Compare the two maps.
  return lexicographical_compare(
      base_to_exponent_map_.cbegin(), base_to_exponent_map_.cend(), mul_e.base_to_exponent_map_.cbegin(),
      mul_e.base_to_exponent_map_.cend(),
      [](const std::pair<const Expression, Expression>& p1, const std::pair<const Expression, Expression>& p2) {
        const Expression& base1{p1.first};
        const Expression& base2{p2.first};
        if (base1.less(base2)) {
          return true;
        }
        if (base2.less(base1)) {
          return false;
        }
        const Expression& exp1{p1.second};
        const Expression& exp2{p2.second};
        return exp1.less(exp2);
      });
}

mpq_class ExpressionMul::Evaluate(const Environment& env) const {
  return std::accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), constant_,
                         [&env](const mpq_class init, const std::pair<const Expression, Expression>& p) {
                           const mpq_class exponent{p.second.Evaluate(env)};
                           if (exponent == 0) {
                             return init;
                           } else if (exponent == 1) {
                             return static_cast<mpq_class>(init * p.first.Evaluate(env));
                           }
                           SMATS_RUNTIME_ERROR("Non 0-1 exponent is not supported in Evaluate.");
                         });
}

Expression ExpressionMul::Expand() const {
  //   (c * ∏ᵢ pow(bᵢ, eᵢ)).Expand()
  // = c * ExpandMultiplication(∏ ExpandPow(bᵢ.Expand(), eᵢ.Expand()))
  return std::accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), Expression{constant_},
                         [](const Expression& init, const std::pair<const Expression, Expression>& p) {
                           const Expression& b_i{p.first};
                           const Expression& e_i{p.second};
                           return ExpandMultiplication(init, ExpandPow(b_i.is_expanded() ? b_i : b_i.Expand(),
                                                                       e_i.is_expanded() ? e_i : e_i.Expand()));
                         });
}

Expression ExpressionMul::EvaluatePartial(const Environment& env) const {
  return accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), Expression{constant_},
                    [&env](const Expression& init, const std::pair<const Expression, Expression>& p) {
                      return init * pow(p.first.EvaluatePartial(env), p.second.EvaluatePartial(env));
                    });
}

Expression ExpressionMul::Substitute(const Substitution& s) const {
  return accumulate(base_to_exponent_map_.begin(), base_to_exponent_map_.end(), Expression{constant_},
                    [&s](const Expression& init, const std::pair<const Expression, Expression>& p) {
                      return init * pow(p.first.Substitute(s), p.second.Substitute(s));
                    });
}

// Computes ∂/∂x pow(f, g).
Expression DifferentiatePow(const Expression& f, const Expression& g, const Variable& x) {
  if (is_constant(g)) {
    const Expression& n{g};  // alias n = g
    // Special case where exponent is a constant:
    //     ∂/∂x pow(f, n) = n * pow(f, n - 1) * ∂/∂x f
    return n * pow(f, n - 1) * f.Differentiate(x);
  }
  if (is_constant(f)) {
    const Expression& n{f};  // alias n = f
    // Special case where base is a constant:
    //     ∂/∂x pow(n, g) = log(n) * pow(n, g) * ∂/∂x g
    return log(n) * pow(n, g) * g.Differentiate(x);
  }
  // General case:
  //    ∂/∂x pow(f, g)
  // = ∂/∂f pow(f, g) * ∂/∂x f + ∂/∂g pow(f, g) * ∂/∂x g
  // = g * pow(f, g - 1) * ∂/∂x f + log(f) * pow(f, g) * ∂/∂x g
  // = pow(f, g - 1) * (g * ∂/∂x f + log(f) * f * ∂/∂x g)
  return pow(f, g - 1) * (g * f.Differentiate(x) + log(f) * f * g.Differentiate(x));
}

Expression ExpressionMul::Differentiate(const Variable& x) const {
  // ∂/∂x (c   * f₁^g₁  * f₂^g₂        * ... * fₙ^gₙ
  //= c * [expr * (∂/∂x f₁^g₁) / f₁^g₁ +
  //       expr * (∂/∂x f₂^g₂) / f₂^g₂ +
  //                      ...          +
  //       expr * (∂/∂x fₙ^gₙ) / fₙ^gₙ]
  // = c * expr * (∑ᵢ (∂/∂x fᵢ^gᵢ) / fᵢ^gᵢ)
  // where expr = (f₁^g₁ * f₂^g₂ * ... * fₙn^gₙ).
  //
  // We distribute (c * expr) into the summation. This possibly cancels the
  // division, "/ fᵢ^gᵢ", and results in a simpler formula.
  //
  // = ∑ᵢ (c * (∂/∂x fᵢ^gᵢ) / fᵢ^gᵢ * expr)

  // This factory will form the expression that we will return.
  ExpressionAddFactory add_fac;
  for (const std::pair<const Expression, Expression>& term : base_to_exponent_map_) {
    const Expression& base{term.first};
    const Expression& exponent{term.second};
    // This factory will form (c * (∂/∂x fᵢ^gᵢ) / fᵢ^gᵢ * expr).
    ExpressionMulFactory mul_fac{constant_, base_to_exponent_map_};
    mul_fac.AddExpression(DifferentiatePow(base, exponent, x));
    mul_fac.AddExpression(pow(base, -exponent));

    add_fac.AddExpression(std::move(mul_fac).GetExpression());
  }
  return std::move(add_fac).GetExpression();
}

std::ostream& ExpressionMul::Display(std::ostream& os) const {
  SMATS_ASSERT(!base_to_exponent_map_.empty());
  bool print_mul{false};
  os << "(";
  if (constant_ != 1.0) {
    os << constant_;
    print_mul = true;
  }
  for (const auto& p : base_to_exponent_map_) {
    DisplayTerm(os, print_mul, p.first, p.second);
    print_mul = true;
  }
  os << ")";
  return os;
}

std::ostream& ExpressionMul::DisplayTerm(std::ostream& os, const bool print_mul, const Expression& base,
                                         const Expression& exponent) const {
  // Print " * pow(base, exponent)" if print_mul is true
  // Print "pow(base, exponent)" if print_mul is false
  // Print "base" instead of "pow(base, exponent)" if exponent == 1.0
  if (print_mul) {
    os << " * ";
  }
  if (is_one(exponent)) {
    os << base;
  } else {
    os << "pow(" << base << ", " << exponent << ")";
  }
  return os;
}

ExpressionMulFactory::ExpressionMulFactory(const mpq_class constant,
                                           std::map<Expression, Expression> base_to_exponent_map)
    : is_expanded_{false}, constant_{constant}, base_to_exponent_map_{std::move(base_to_exponent_map)} {}

ExpressionMulFactory::ExpressionMulFactory(const std::map<Variable, int>& base_to_exponent_map)
    : is_expanded_{true}, constant_{1.0} {
  for (const auto& [base, exponent] : base_to_exponent_map) {
    base_to_exponent_map_.emplace(base, exponent);
  }
}

ExpressionMulFactory::ExpressionMulFactory(const ExpressionMul& mul)
    : ExpressionMulFactory{mul.get_constant(), mul.get_base_to_exponent_map()} {
  is_expanded_ = mul.is_expanded();
}

void ExpressionMulFactory::AddExpression(const Expression& e) {
  if (constant_ == 0.0) {
    return;  // Do nothing if it already represented 0.
  }
  if (is_zero(e)) {
    // X * 0 => 0. So clear the constant and the map.
    return SetZero();
  }
  if (is_constant(e)) {
    return AddConstant(get_constant_value(e));
  }
  if (is_multiplication(e)) {
    // Flattening
    return Add(to_multiplication(e));
  }
  // Add e^1
  return AddTerm(e, Expression{1.0});
}

void ExpressionMulFactory::ADD(const ExpressionMul& mul) {
  if (constant_ == 0.0) {
    return;  // Do nothing if it already represented 0.
  }
  AddConstant(mul.get_constant());
  AddMap(mul.get_base_to_exponent_map());
}

void ExpressionMulFactory::SetZero() {
  is_expanded_ = true;
  constant_ = 0.0;
  base_to_exponent_map_.clear();
}

ExpressionMulFactory& ExpressionMulFactory::operator=(const ExpressionMul& mul) {
  is_expanded_ = mul.is_expanded();
  constant_ = mul.get_constant();
  base_to_exponent_map_ = mul.get_base_to_exponent_map();
  return *this;
}

ExpressionMulFactory&& ExpressionMulFactory::Negate() && {
  constant_ = -constant_;
  return std::move(*this);
}

Expression ExpressionMulFactory::GetExpression() && {
  if (base_to_exponent_map_.empty()) {
    return Expression{constant_};
  }
  if (constant_ == 1.0 && base_to_exponent_map_.size() == 1U) {
    // 1.0 * c1^t1 -> c1^t1
    const auto it(base_to_exponent_map_.cbegin());
    return pow(it->first, it->second);
  }
  auto result = make_unique<ExpressionMul>(constant_, std::move(base_to_exponent_map_));
  if (is_expanded_) {
    result->set_expanded();
  }
  return Expression{std::move(result)};
}

void ExpressionMulFactory::AddConstant(const mpq_class constant) {
  if (constant == 0.0) {
    return SetZero();
  }
  constant_ *= constant;
}

void ExpressionMulFactory::AddTerm(const Expression& base, const Expression& exponent) {
  // The following assertion holds because of
  // ExpressionMulFactory::AddExpression.
  SMATS_ASSERT(!(is_constant(base) && is_constant(exponent)));
  if (is_pow(base)) {
    // If (base, exponent) = (pow(e1, e2), exponent)), then add (e1, e2 *
    // exponent)
    // Example: (x^2)^3 => x^(2 * 3)
    return AddTerm(get_first_argument(base), get_second_argument(base) * exponent);
  }

  const auto it(base_to_exponent_map_.find(base));
  if (it != base_to_exponent_map_.end()) {
    // base is already in map.
    // (= b1^e1 * ... * (base^this_exponent) * ... * en^bn).
    // Update it to be (... * (base^(this_exponent + exponent)) * ...)
    // Example: x^3 * x^2 => x^5
    Expression& this_exponent = it->second;
    this_exponent += exponent;
    if (is_zero(this_exponent)) {
      // If it ends up with base^0 (= 1.0) then remove this entry from the map.
      // TODO(soonho-tri): The following operation is not sound since it can
      // cancels `base` which might include 0/0 problems.
      base_to_exponent_map_.erase(it);
    } else {
      // Conservatively, when adjusting an exponent we can no longer be sure
      // that our ExpressionMul remains in expanded form. Note that it's very
      // difficult to find a unit test where this line matters (is_expanded_
      // is almost always "false" here already), but in any case setting this
      // to false here is conservative (at worst, a performance loss).
      is_expanded_ = false;
    }
  } else {
    // Product is not found in base_to_exponent_map_. Add the entry (base,
    // exponent).
    base_to_exponent_map_.emplace(base, exponent);
    // Conservatively, unless the new term is a simple exponentiation between
    // leaf expressions (i.e., constants or variables, not any cell type that
    // nests another Expression inside of itself), we can no longer be sure
    // that our ExpressionMul remains in expanded form.
    if (!(IsLeafExpression(base) && IsLeafExpression(exponent))) {
      is_expanded_ = false;
    }
  }
}

void ExpressionMulFactory::AddMap(const std::map<Expression, Expression>& base_to_exponent_map) {
  for (const auto& p : base_to_exponent_map) {
    AddTerm(p.first, p.second);
  }
}

ExpressionDiv::ExpressionDiv(const Expression& e1, const Expression& e2)
    : BinaryExpressionCell{ExpressionKind::Div, e1, e2, e1.is_polynomial() && is_constant(e2), false} {}

namespace {
// Helper class to implement ExpressionDiv::Expand. Given a symbolic expression
// `e` and a constant `n`, it pushes the division in `e / n` inside for the
// following cases:
//
// Case Addition      :      (c₀ + ∑ᵢ (cᵢ * eᵢ)) / n
//                        => c₀/n + ∑ᵢ (cᵢ / n * eᵢ)
//
// Case Multiplication:      (c₀ * ∏ᵢ (bᵢ * eᵢ)) / n
//                        => c₀ / n * ∏ᵢ (bᵢ * eᵢ)
//
// Case Division      :      (e₁ / m) / n
//                        => Recursively simplify e₁ / (n * m)
//
//                           (e₁ / e₂) / n
//                        =  (e₁ / n) / e₂
//                        => Recursively simplify (e₁ / n) and divide it by e₂
//
// Other cases        :      e / n
//                        => (1/n) * e
//
// Note that we use VisitExpression instead of VisitPolynomial because we want
// to handle cases such as `(6xy / z) / 3` where (6xy / z) is not a polynomial
// but it's desirable to simplify the expression into `2xy / z`.
class DivExpandVisitor {
 public:
  [[nodiscard]] Expression Simplify(const Expression& e, const mpq_class n) const {
    return VisitExpression<Expression>(this, e, n);
  }

 private:
  [[nodiscard]] Expression VisitAddition(const Expression& e, const mpq_class n) const {
    // e =  (c₀ + ∑ᵢ (cᵢ * eᵢ)) / n
    //   => c₀/n + ∑ᵢ (cᵢ / n * eᵢ)
    const mpq_class constant{get_constant_in_addition(e)};
    ExpressionAddFactory factory(constant / n, {});
    for (const std::pair<const Expression, mpq_class>& p : get_expr_to_coeff_map_in_addition(e)) {
      factory.AddExpression(p.second / n * p.first);
    }
    return std::move(factory).GetExpression();
  }
  [[nodiscard]] Expression VisitMultiplication(const Expression& e, const mpq_class n) const {
    // e =  (c₀ * ∏ᵢ (bᵢ * eᵢ)) / n
    //   => c₀ / n * ∏ᵢ (bᵢ * eᵢ)
    return ExpressionMulFactory{get_constant_in_multiplication(e) / n, get_base_to_exponent_map_in_multiplication(e)}
        .GetExpression();
  }
  [[nodiscard]] Expression VisitDivision(const Expression& e, const mpq_class n) const {
    const Expression& e1{get_first_argument(e)};
    const Expression& e2{get_second_argument(e)};
    if (is_constant(e2)) {
      // e =  (e₁ / m) / n
      //   => Simplify `e₁ / (n * m)`
      const mpq_class m{get_constant_value(e2)};
      return Simplify(e1, m * n);
    } else {
      // e =  (e₁ / e₂) / n
      //   => (e₁ / n) / e₂
      return Simplify(e1, n) / e2;
    }
  }
  [[nodiscard]] Expression VisitVariable(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitConstant(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitLog(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitPow(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitAbs(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitExp(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitSqrt(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitSin(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitCos(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitTan(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitAsin(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitAcos(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitAtan(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitAtan2(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitSinh(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitCosh(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitTanh(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitMin(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitMax(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitCeil(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitFloor(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitIfThenElse(const Expression& e, const mpq_class n) const { return (1.0 / n) * e; }
  [[nodiscard]] Expression VisitUninterpretedFunction(const Expression& e, const mpq_class n) const {
    return (1.0 / n) * e;
  }

  // Makes VisitExpression a friend of this class so that VisitExpression can
  // use its private methods.
  friend Expression smats::VisitExpression<Expression>(const DivExpandVisitor*, const Expression&,
                                                                 const mpq_class&);
};
}  // namespace

Expression ExpressionDiv::Expand() const {
  const Expression& first{get_first_argument()};
  const Expression& second{get_second_argument()};
  const Expression e1{first.is_expanded() ? first : first.Expand()};
  const Expression e2{second.is_expanded() ? second : second.Expand()};
  if (is_constant(e2)) {
    // Simplifies the 'division by a constant' case, using DivExpandVisitor
    // defined above.
    return DivExpandVisitor{}.Simplify(e1, get_constant_value(e2));
  } else {
    return (e1 / e2);
  }
}

Expression ExpressionDiv::EvaluatePartial(const Environment& env) const {
  return get_first_argument().EvaluatePartial(env) / get_second_argument().EvaluatePartial(env);
}

Expression ExpressionDiv::Substitute(const Substitution& s) const {
  return get_first_argument().Substitute(s) / get_second_argument().Substitute(s);
}

Expression ExpressionDiv::Differentiate(const Variable& x) const {
  // ∂/∂x (f / g) = (∂/∂x f * g - f * ∂/∂x g) / g^2
  const Expression& f{get_first_argument()};
  const Expression& g{get_second_argument()};
  return (f.Differentiate(x) * g - f * g.Differentiate(x)) / pow(g, 2.0);
}

std::ostream& ExpressionDiv::Display(std::ostream& os) const {
  return os << "(" << get_first_argument() << " / " << get_second_argument() << ")";
}

mpq_class ExpressionDiv::DoEvaluate(const mpq_class v1, const mpq_class v2) const {
  if (v2 == 0.0) {
    std::ostringstream oss;
    oss << "Division by zero: " << v1 << " / " << v2;
    this->Display(oss) << std::endl;
    throw std::runtime_error(oss.str());
  }
  return v1 / v2;
}

ExpressionLog::ExpressionLog(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Log, e, false, e.is_expanded()} {}

void ExpressionLog::check_domain(const mpq_class v) {
  if (!(v >= 0)) {
    std::ostringstream oss;
    oss << "log(" << v << ") : numerical argument out of domain. " << v << " is not in [0, +oo)" << std::endl;
    throw std::domain_error(oss.str());
  }
}

Expression ExpressionLog::Expand() const {
  const Expression& arg{get_argument()};
  return log(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionLog::EvaluatePartial(const Environment& env) const {
  return log(get_argument().EvaluatePartial(env));
}

Expression ExpressionLog::Substitute(const Substitution& s) const { return log(get_argument().Substitute(s)); }

Expression ExpressionLog::Differentiate(const Variable& x) const {
  // ∂/∂x log(f) = (∂/∂x f) / f
  const Expression& f{get_argument()};
  return f.Differentiate(x) / f;
}

std::ostream& ExpressionLog::Display(std::ostream& os) const { return os << "log(" << get_argument() << ")"; }

mpq_class ExpressionLog::DoEvaluate(const mpq_class v) const {
  SMATS_RUNTIME_ERROR("DoEvaluate is not supported for log.");
#if 0
  check_domain(v);
  return std::log(v);
#endif
}

ExpressionAbs::ExpressionAbs(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Abs, e, false, e.is_expanded()} {}

Expression ExpressionAbs::Expand() const {
  const Expression& arg{get_argument()};
  return abs(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionAbs::EvaluatePartial(const Environment& env) const {
  return abs(get_argument().EvaluatePartial(env));
}

Expression ExpressionAbs::Substitute(const Substitution& s) const { return abs(get_argument().Substitute(s)); }

Expression ExpressionAbs::Differentiate(const Variable& x) const {
  if (variables().include(x)) {
    const Expression& arg{get_argument()};
    const Expression deriv = arg.Differentiate(x);
    return if_then_else(arg < 0, -deriv, if_then_else(arg == 0, Expression::NaN(), deriv));
  } else {
    return Expression::Zero();
  }
}

std::ostream& ExpressionAbs::Display(std::ostream& os) const { return os << "abs(" << get_argument() << ")"; }

mpq_class ExpressionAbs::DoEvaluate(const mpq_class v) const { return std::fabs(v); }

ExpressionExp::ExpressionExp(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Exp, e, false, e.is_expanded()} {}

Expression ExpressionExp::Expand() const {
  const Expression& arg{get_argument()};
  return exp(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionExp::EvaluatePartial(const Environment& env) const {
  return exp(get_argument().EvaluatePartial(env));
}

Expression ExpressionExp::Substitute(const Substitution& s) const { return exp(get_argument().Substitute(s)); }

Expression ExpressionExp::Differentiate(const Variable& x) const {
  // ∂/∂x exp(f) = exp(f) * (∂/∂x f)
  const Expression& f{get_argument()};
  return exp(f) * f.Differentiate(x);
}

std::ostream& ExpressionExp::Display(std::ostream& os) const { return os << "exp(" << get_argument() << ")"; }

mpq_class ExpressionExp::DoEvaluate(const mpq_class v) const { return std::exp(v); }

ExpressionSqrt::ExpressionSqrt(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Sqrt, e, false, e.is_expanded()} {}

void ExpressionSqrt::check_domain(const mpq_class v) {
  if (!(v >= 0)) {
    std::ostringstream oss;
    oss << "sqrt(" << v << ") : numerical argument out of domain. " << v << " is not in [0, +oo)" << std::endl;
    throw std::domain_error(oss.str());
  }
}

Expression ExpressionSqrt::Expand() const {
  const Expression& arg{get_argument()};
  return sqrt(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionSqrt::EvaluatePartial(const Environment& env) const {
  return sqrt(get_argument().EvaluatePartial(env));
}

Expression ExpressionSqrt::Substitute(const Substitution& s) const { return sqrt(get_argument().Substitute(s)); }

Expression ExpressionSqrt::Differentiate(const Variable& x) const {
  // ∂/∂x (sqrt(f)) = 1 / (2 * sqrt(f)) * (∂/∂x f)
  const Expression& f{get_argument()};
  return 1 / (2 * sqrt(f)) * f.Differentiate(x);
}

std::ostream& ExpressionSqrt::Display(std::ostream& os) const { return os << "sqrt(" << get_argument() << ")"; }

mpq_class ExpressionSqrt::DoEvaluate(const mpq_class v) const {
  check_domain(v);
  return std::sqrt(v);
}

ExpressionPow::ExpressionPow(const Expression& e1, const Expression& e2)
    : BinaryExpressionCell{ExpressionKind::Pow, e1, e2, determine_polynomial(e1, e2),
                           IsLeafExpression(e1) && IsLeafExpression(e2)} {}

void ExpressionPow::check_domain(const mpq_class v1, const mpq_class v2) {
  if (std::isfinite(v1) && (v1 < 0.0) && std::isfinite(v2) && !is_integer(v2)) {
    std::ostringstream oss;
    oss << "pow(" << v1 << ", " << v2 << ") : numerical argument out of domain. " << v1 << " is finite negative and "
        << v2 << " is finite non-integer." << std::endl;
    throw std::domain_error(oss.str());
  }
}

Expression ExpressionPow::Expand() const {
  const Expression& e1{get_first_argument()};
  const Expression& e2{get_second_argument()};
  return ExpandPow(e1.is_expanded() ? e1 : e1.Expand(), e2.is_expanded() ? e2 : e2.Expand());
}

Expression ExpressionPow::EvaluatePartial(const Environment& env) const {
  return pow(get_first_argument().EvaluatePartial(env), get_second_argument().EvaluatePartial(env));
}

Expression ExpressionPow::Substitute(const Substitution& s) const {
  return pow(get_first_argument().Substitute(s), get_second_argument().Substitute(s));
}

Expression ExpressionPow::Differentiate(const Variable& x) const {
  return DifferentiatePow(get_first_argument(), get_second_argument(), x);
}

std::ostream& ExpressionPow::Display(std::ostream& os) const {
  return os << "pow(" << get_first_argument() << ", " << get_second_argument() << ")";
}

mpq_class ExpressionPow::DoEvaluate(const mpq_class v1, const mpq_class v2) const {
  check_domain(v1, v2);
  return std::pow(v1, v2);
}

ExpressionSin::ExpressionSin(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Sin, e, false, e.is_expanded()} {}

Expression ExpressionSin::Expand() const {
  const Expression& arg{get_argument()};
  return sin(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionSin::EvaluatePartial(const Environment& env) const {
  return sin(get_argument().EvaluatePartial(env));
}

Expression ExpressionSin::Substitute(const Substitution& s) const { return sin(get_argument().Substitute(s)); }

Expression ExpressionSin::Differentiate(const Variable& x) const {
  // ∂/∂x (sin f) = (cos f) * (∂/∂x f)
  const Expression& f{get_argument()};
  return cos(f) * f.Differentiate(x);
}

std::ostream& ExpressionSin::Display(std::ostream& os) const { return os << "sin(" << get_argument() << ")"; }

mpq_class ExpressionSin::DoEvaluate(const mpq_class v) const { return std::sin(v); }

ExpressionCos::ExpressionCos(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Cos, e, false, e.is_expanded()} {}

Expression ExpressionCos::Expand() const {
  const Expression& arg{get_argument()};
  return cos(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionCos::EvaluatePartial(const Environment& env) const {
  return cos(get_argument().EvaluatePartial(env));
}

Expression ExpressionCos::Substitute(const Substitution& s) const { return cos(get_argument().Substitute(s)); }

Expression ExpressionCos::Differentiate(const Variable& x) const {
  // ∂/∂x (cos f) = - (sin f) * (∂/∂x f)
  const Expression& f{get_argument()};
  return -sin(f) * f.Differentiate(x);
}

std::ostream& ExpressionCos::Display(std::ostream& os) const { return os << "cos(" << get_argument() << ")"; }

mpq_class ExpressionCos::DoEvaluate(const mpq_class v) const { return std::cos(v); }

ExpressionTan::ExpressionTan(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Tan, e, false, e.is_expanded()} {}

Expression ExpressionTan::Expand() const {
  const Expression& arg{get_argument()};
  return tan(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionTan::EvaluatePartial(const Environment& env) const {
  return tan(get_argument().EvaluatePartial(env));
}

Expression ExpressionTan::Substitute(const Substitution& s) const { return tan(get_argument().Substitute(s)); }

Expression ExpressionTan::Differentiate(const Variable& x) const {
  // ∂/∂x (tan f) = (1 / (cos f)^2) * (∂/∂x f)
  const Expression& f{get_argument()};
  return (1 / pow(cos(f), 2)) * f.Differentiate(x);
}

std::ostream& ExpressionTan::Display(std::ostream& os) const { return os << "tan(" << get_argument() << ")"; }

mpq_class ExpressionTan::DoEvaluate(const mpq_class v) const { return std::tan(v); }

ExpressionAsin::ExpressionAsin(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Asin, e, false, e.is_expanded()} {}

void ExpressionAsin::check_domain(const mpq_class v) {
  if (!((v >= -1.0) && (v <= 1.0))) {
    std::ostringstream oss;
    oss << "asin(" << v << ") : numerical argument out of domain. " << v << " is not in [-1.0, +1.0]" << std::endl;
    throw std::domain_error(oss.str());
  }
}

Expression ExpressionAsin::Expand() const {
  const Expression& arg{get_argument()};
  return asin(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionAsin::EvaluatePartial(const Environment& env) const {
  return asin(get_argument().EvaluatePartial(env));
}

Expression ExpressionAsin::Substitute(const Substitution& s) const { return asin(get_argument().Substitute(s)); }

Expression ExpressionAsin::Differentiate(const Variable& x) const {
  // ∂/∂x (asin f) = (1 / sqrt(1 - f^2)) (∂/∂x f)
  const Expression& f{get_argument()};
  return (1 / sqrt(1 - pow(f, 2))) * f.Differentiate(x);
}

std::ostream& ExpressionAsin::Display(std::ostream& os) const { return os << "asin(" << get_argument() << ")"; }

mpq_class ExpressionAsin::DoEvaluate(const mpq_class v) const {
  check_domain(v);
  return std::asin(v);
}

ExpressionAcos::ExpressionAcos(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Acos, e, false, e.is_expanded()} {}

void ExpressionAcos::check_domain(const mpq_class v) {
  if (!((v >= -1.0) && (v <= 1.0))) {
    std::ostringstream oss;
    oss << "acos(" << v << ") : numerical argument out of domain. " << v << " is not in [-1.0, +1.0]" << std::endl;
    throw std::domain_error(oss.str());
  }
}

Expression ExpressionAcos::Expand() const {
  const Expression& arg{get_argument()};
  return acos(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionAcos::EvaluatePartial(const Environment& env) const {
  return acos(get_argument().EvaluatePartial(env));
}

Expression ExpressionAcos::Substitute(const Substitution& s) const { return acos(get_argument().Substitute(s)); }

Expression ExpressionAcos::Differentiate(const Variable& x) const {
  // ∂/∂x (acos f) = - 1 / sqrt(1 - f^2) * (∂/∂x f)
  const Expression& f{get_argument()};
  return -1 / sqrt(1 - pow(f, 2)) * f.Differentiate(x);
}

std::ostream& ExpressionAcos::Display(std::ostream& os) const { return os << "acos(" << get_argument() << ")"; }

mpq_class ExpressionAcos::DoEvaluate(const mpq_class v) const {
  check_domain(v);
  return std::acos(v);
}

ExpressionAtan::ExpressionAtan(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Atan, e, false, e.is_expanded()} {}

Expression ExpressionAtan::Expand() const {
  const Expression& arg{get_argument()};
  return atan(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionAtan::EvaluatePartial(const Environment& env) const {
  return atan(get_argument().EvaluatePartial(env));
}

Expression ExpressionAtan::Substitute(const Substitution& s) const { return atan(get_argument().Substitute(s)); }

Expression ExpressionAtan::Differentiate(const Variable& x) const {
  // ∂/∂x (atan f) = (1 / (1 + f^2)) * ∂/∂x f
  const Expression& f{get_argument()};
  return (1 / (1 + pow(f, 2))) * f.Differentiate(x);
}

std::ostream& ExpressionAtan::Display(std::ostream& os) const { return os << "atan(" << get_argument() << ")"; }

mpq_class ExpressionAtan::DoEvaluate(const mpq_class v) const { return std::atan(v); }

ExpressionAtan2::ExpressionAtan2(const Expression& e1, const Expression& e2)
    : BinaryExpressionCell{ExpressionKind::Atan2, e1, e2, false, e1.is_expanded() && e2.is_expanded()} {}

Expression ExpressionAtan2::Expand() const {
  const Expression& e1{get_first_argument()};
  const Expression& e2{get_second_argument()};
  return atan2(e1.is_expanded() ? e1 : e1.Expand(), e2.is_expanded() ? e2 : e2.Expand());
}

Expression ExpressionAtan2::EvaluatePartial(const Environment& env) const {
  return atan2(get_first_argument().EvaluatePartial(env), get_second_argument().EvaluatePartial(env));
}

Expression ExpressionAtan2::Substitute(const Substitution& s) const {
  return atan2(get_first_argument().Substitute(s), get_second_argument().Substitute(s));
}

Expression ExpressionAtan2::Differentiate(const Variable& x) const {
  // ∂/∂x (atan2(f,g)) = (g * (∂/∂x f) - f * (∂/∂x g)) / (f^2 + g^2)
  const Expression& f{get_first_argument()};
  const Expression& g{get_second_argument()};
  return (g * f.Differentiate(x) - f * g.Differentiate(x)) / (pow(f, 2) + pow(g, 2));
}

std::ostream& ExpressionAtan2::Display(std::ostream& os) const {
  return os << "atan2(" << get_first_argument() << ", " << get_second_argument() << ")";
}

mpq_class ExpressionAtan2::DoEvaluate(const mpq_class v1, const mpq_class v2) const { return std::atan2(v1, v2); }

ExpressionSinh::ExpressionSinh(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Sinh, e, false, e.is_expanded()} {}

Expression ExpressionSinh::Expand() const {
  const Expression& arg{get_argument()};
  return sinh(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionSinh::EvaluatePartial(const Environment& env) const {
  return sinh(get_argument().EvaluatePartial(env));
}

Expression ExpressionSinh::Substitute(const Substitution& s) const { return sinh(get_argument().Substitute(s)); }

Expression ExpressionSinh::Differentiate(const Variable& x) const {
  // ∂/∂x (sinh f) = cosh(f) * (∂/∂x f)
  const Expression& f{get_argument()};
  return cosh(f) * f.Differentiate(x);
}

std::ostream& ExpressionSinh::Display(std::ostream& os) const { return os << "sinh(" << get_argument() << ")"; }

mpq_class ExpressionSinh::DoEvaluate(const mpq_class v) const { return std::sinh(v); }

ExpressionCosh::ExpressionCosh(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Cosh, e, false, e.is_expanded()} {}

Expression ExpressionCosh::Expand() const {
  const Expression& arg{get_argument()};
  return cosh(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionCosh::EvaluatePartial(const Environment& env) const {
  return cosh(get_argument().EvaluatePartial(env));
}

Expression ExpressionCosh::Substitute(const Substitution& s) const { return cosh(get_argument().Substitute(s)); }

Expression ExpressionCosh::Differentiate(const Variable& x) const {
  // ∂/∂x (cosh f) = sinh(f) * (∂/∂x f)
  const Expression& f{get_argument()};
  return sinh(f) * f.Differentiate(x);
}

std::ostream& ExpressionCosh::Display(std::ostream& os) const { return os << "cosh(" << get_argument() << ")"; }

mpq_class ExpressionCosh::DoEvaluate(const mpq_class v) const { return std::cosh(v); }

ExpressionTanh::ExpressionTanh(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Tanh, e, false, e.is_expanded()} {}

Expression ExpressionTanh::Expand() const {
  const Expression& arg{get_argument()};
  return tanh(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionTanh::EvaluatePartial(const Environment& env) const {
  return tanh(get_argument().EvaluatePartial(env));
}

Expression ExpressionTanh::Substitute(const Substitution& s) const { return tanh(get_argument().Substitute(s)); }

Expression ExpressionTanh::Differentiate(const Variable& x) const {
  // ∂/∂x (tanh f) = 1 / (cosh^2(f)) * (∂/∂x f)
  const Expression& f{get_argument()};
  return 1 / pow(cosh(f), 2) * f.Differentiate(x);
}

std::ostream& ExpressionTanh::Display(std::ostream& os) const { return os << "tanh(" << get_argument() << ")"; }

mpq_class ExpressionTanh::DoEvaluate(const mpq_class v) const { return std::tanh(v); }

ExpressionMin::ExpressionMin(const Expression& e1, const Expression& e2)
    : BinaryExpressionCell{ExpressionKind::Min, e1, e2, false, e1.is_expanded() && e2.is_expanded()} {}

Expression ExpressionMin::Expand() const {
  const Expression& e1{get_first_argument()};
  const Expression& e2{get_second_argument()};
  return min(e1.is_expanded() ? e1 : e1.Expand(), e2.is_expanded() ? e2 : e2.Expand());
}

Expression ExpressionMin::EvaluatePartial(const Environment& env) const {
  return min(get_first_argument().EvaluatePartial(env), get_second_argument().EvaluatePartial(env));
}

Expression ExpressionMin::Substitute(const Substitution& s) const {
  return min(get_first_argument().Substitute(s), get_second_argument().Substitute(s));
}

Expression ExpressionMin::Differentiate(const Variable& x) const {
  if (variables().include(x)) {
    const Expression& e1{get_first_argument()};
    const Expression& e2{get_second_argument()};
    return if_then_else(e1 < e2, e1.Differentiate(x), if_then_else(e1 == e2, Expression::NaN(), e2.Differentiate(x)));
  } else {
    return Expression::Zero();
  }
}

std::ostream& ExpressionMin::Display(std::ostream& os) const {
  return os << "min(" << get_first_argument() << ", " << get_second_argument() << ")";
}

mpq_class ExpressionMin::DoEvaluate(const mpq_class v1, const mpq_class v2) const { return std::min(v1, v2); }

ExpressionMax::ExpressionMax(const Expression& e1, const Expression& e2)
    : BinaryExpressionCell{ExpressionKind::Max, e1, e2, false, e1.is_expanded() && e2.is_expanded()} {}

Expression ExpressionMax::Expand() const {
  const Expression& e1{get_first_argument()};
  const Expression& e2{get_second_argument()};
  return max(e1.is_expanded() ? e1 : e1.Expand(), e2.is_expanded() ? e2 : e2.Expand());
}

Expression ExpressionMax::EvaluatePartial(const Environment& env) const {
  return max(get_first_argument().EvaluatePartial(env), get_second_argument().EvaluatePartial(env));
}

Expression ExpressionMax::Substitute(const Substitution& s) const {
  return max(get_first_argument().Substitute(s), get_second_argument().Substitute(s));
}

Expression ExpressionMax::Differentiate(const Variable& x) const {
  if (variables().include(x)) {
    const Expression& e1{get_first_argument()};
    const Expression& e2{get_second_argument()};
    return if_then_else(e1 > e2, e1.Differentiate(x), if_then_else(e1 == e2, Expression::NaN(), e2.Differentiate(x)));
  } else {
    return Expression::Zero();
  }
}

std::ostream& ExpressionMax::Display(std::ostream& os) const {
  return os << "max(" << get_first_argument() << ", " << get_second_argument() << ")";
}

mpq_class ExpressionMax::DoEvaluate(const mpq_class v1, const mpq_class v2) const { return std::max(v1, v2); }

ExpressionCeiling::ExpressionCeiling(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Ceil, e, false, e.is_expanded()} {}

Expression ExpressionCeiling::Expand() const {
  const Expression& arg{get_argument()};
  return ceil(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionCeiling::EvaluatePartial(const Environment& env) const {
  return ceil(get_argument().EvaluatePartial(env));
}

Expression ExpressionCeiling::Substitute(const Substitution& s) const { return ceil(get_argument().Substitute(s)); }

Expression ExpressionCeiling::Differentiate(const Variable& x) const {
  if (variables().include(x)) {
    const Expression& arg{get_argument()};
    // FYI:  'ceil(x) == floor(x)` is the same as `x % 1 == 0`.
    return if_then_else(ceil(arg) == floor(arg), Expression::NaN(), Expression::Zero());
  } else {
    return Expression::Zero();
  }
}

std::ostream& ExpressionCeiling::Display(std::ostream& os) const { return os << "ceil(" << get_argument() << ")"; }

mpq_class ExpressionCeiling::DoEvaluate(const mpq_class v) const { return std::ceil(v); }

ExpressionFloor::ExpressionFloor(const Expression& e)
    : UnaryExpressionCell{ExpressionKind::Floor, e, false, e.is_expanded()} {}

Expression ExpressionFloor::Expand() const {
  const Expression& arg{get_argument()};
  return floor(arg.is_expanded() ? arg : arg.Expand());
}

Expression ExpressionFloor::EvaluatePartial(const Environment& env) const {
  return floor(get_argument().EvaluatePartial(env));
}

Expression ExpressionFloor::Substitute(const Substitution& s) const { return floor(get_argument().Substitute(s)); }

Expression ExpressionFloor::Differentiate(const Variable& x) const {
  if (variables().include(x)) {
    const Expression& arg{get_argument()};
    // FYI:  'ceil(x) == floor(x)` is the same as `x % 1 == 0`.
    return if_then_else(ceil(arg) == floor(arg), Expression::NaN(), Expression::Zero());
  } else {
    return Expression::Zero();
  }
}

std::ostream& ExpressionFloor::Display(std::ostream& os) const { return os << "floor(" << get_argument() << ")"; }

mpq_class ExpressionFloor::DoEvaluate(const mpq_class v) const { return std::floor(v); }

// ExpressionIfThenElse
// --------------------
ExpressionIfThenElse::ExpressionIfThenElse(Formula f_cond, Expression e_then, Expression e_else)
    : ExpressionCell{ExpressionKind::IfThenElse, false, false},
      f_cond_{std::move(f_cond)},
      e_then_{std::move(e_then)},
      e_else_{std::move(e_else)} {}

void ExpressionIfThenElse::hash(DelegatingHasher& hasher) const {
  using smats::hash_append;
  hash_append(*hasher, f_cond_);
  hash_append(*hasher, e_then_);
  hash_append(*hasher, e_else_);
}

Variables ExpressionIfThenElse::variables() const {
  Variables ret{f_cond_.GetFreeVariables()};
  ret.insert(e_then_.variables());
  ret.insert(e_else_.variables());
  return ret;
}

bool ExpressionIfThenElse::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionIfThenElse& ite_e{static_cast<const ExpressionIfThenElse&>(e)};
  return f_cond_.equal_to(ite_e.f_cond_) && e_then_.equal_to(ite_e.e_then_) && e_else_.equal_to(ite_e.e_else_);
}

bool ExpressionIfThenElse::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionIfThenElse& ite_e{static_cast<const ExpressionIfThenElse&>(e)};
  if (f_cond_.less(ite_e.f_cond_)) {
    return true;
  }
  if (ite_e.f_cond_.less(f_cond_)) {
    return false;
  }
  if (e_then_.less(ite_e.e_then_)) {
    return true;
  }
  if (ite_e.e_then_.less(e_then_)) {
    return false;
  }
  return e_else_.less(ite_e.e_else_);
}

mpq_class ExpressionIfThenElse::Evaluate(const Environment& env) const {
  if (f_cond_.Evaluate(env)) {
    return e_then_.Evaluate(env);
  }
  return e_else_.Evaluate(env);
}

Expression ExpressionIfThenElse::Expand() const {
  // TODO(soonho): use the following line when Formula::Expand() is implemented.
  // return if_then_else(f_cond_.Expand(), e_then_.Expand(), e_else_.Expand());
  throw std::runtime_error("Not yet implemented.");
}

Expression ExpressionIfThenElse::EvaluatePartial(const Environment& env) const {
  // TODO(jwnimmer-tri) We could define a Formula::EvaluatePartial for improved
  // performance, if necessary.
  Substitution subst;
  for (const std::pair<const Variable, mpq_class>& p : env) {
    subst.emplace(p.first, p.second);
  }
  return if_then_else(f_cond_.Substitute(subst), e_then_.EvaluatePartial(env), e_else_.EvaluatePartial(env));
}

Expression ExpressionIfThenElse::Substitute(const Substitution& s) const {
  return if_then_else(f_cond_.Substitute(s), e_then_.Substitute(s), e_else_.Substitute(s));
}

Expression ExpressionIfThenElse::Differentiate(const Variable& x) const {
  if (variables().include(x)) {
    if (is_relational(f_cond_)) {
      // In relational formulae, the discontinuity is at lhs == rhs.
      // TODO(ggould-tri) The logic of where/whether to find discontinuities
      // in a `Formula` belongs in that class, not here.  That could also
      // handle eg the degenerate case of constant formulae.
      // Refer to #8648 for additional information.
      return if_then_else(get_lhs_expression(f_cond_) == get_rhs_expression(f_cond_), Expression::NaN(),
                          if_then_else(f_cond_, e_then_.Differentiate(x), e_else_.Differentiate(x)));
    } else {
      // Because we cannot write an expression for whether the condition is
      // discontinuous at a given environment, we blanket disallow
      // differentiation where the condition contains the differentiand.  We
      // hope that users can generally avoid this in practice, eg by using min
      // and max instead.
      std::ostringstream oss;
      Display(oss) << " is not differentiable with respect to " << x << ".";
      throw std::runtime_error(oss.str());
    }
  } else {
    return Expression::Zero();
  }
}

std::ostream& ExpressionIfThenElse::Display(std::ostream& os) const {
  return os << "(if " << f_cond_ << " then " << e_then_ << " else " << e_else_ << ")";
}

// ExpressionUninterpretedFunction
// --------------------
ExpressionUninterpretedFunction::ExpressionUninterpretedFunction(string name, std::vector<Expression> arguments)
    : ExpressionCell{ExpressionKind::UninterpretedFunction, false,
                     all_of(arguments.begin(), arguments.end(),
                            [](const Expression& arg) { return arg.is_expanded(); })},
      name_{std::move(name)},
      arguments_{std::move(arguments)} {}

void ExpressionUninterpretedFunction::hash(DelegatingHasher& hasher) const {
  using smats::hash_append;
  hash_append(*hasher, name_);
  hash_append_range(*hasher, arguments_.begin(), arguments_.end());
}

Variables ExpressionUninterpretedFunction::variables() const {
  Variables ret;
  for (const Expression& arg : arguments_) {
    ret += arg.variables();
  }
  return ret;
}

bool ExpressionUninterpretedFunction::equal_to(const ExpressionCell& e) const {
  // Expression::equal_to guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionUninterpretedFunction& uf_e{static_cast<const ExpressionUninterpretedFunction&>(e)};
  return name_ == uf_e.name_ &&
         equal(arguments_.begin(), arguments_.end(), uf_e.arguments_.begin(), uf_e.arguments_.end(),
               [](const Expression& e1, const Expression& e2) { return e1.equal_to(e2); });
}

bool ExpressionUninterpretedFunction::less(const ExpressionCell& e) const {
  // Expression::less guarantees the following assertion.
  SMATS_ASSERT(kind() == e.kind(), "Both expressions must have the same kind");
  const ExpressionUninterpretedFunction& uf_e{static_cast<const ExpressionUninterpretedFunction&>(e)};
  if (name_ < uf_e.name_) {
    return true;
  }
  if (uf_e.name_ < name_) {
    return false;
  }
  return lexicographical_compare(arguments_.begin(), arguments_.end(), uf_e.arguments_.begin(), uf_e.arguments_.end(),
                                 [](const Expression& e1, const Expression& e2) { return e1.less(e2); });
}

mpq_class ExpressionUninterpretedFunction::Evaluate(const Environment&) const {
  throw std::runtime_error("Uninterpreted-function expression cannot be evaluated.");
}

Expression ExpressionUninterpretedFunction::Expand() const {
  std::vector<Expression> new_arguments;
  new_arguments.reserve(arguments_.size());
  for (const Expression& arg : arguments_) {
    new_arguments.push_back(arg.is_expanded() ? arg : arg.Expand());
  }
  return uninterpreted_function(name_, std::move(new_arguments));
}

Expression ExpressionUninterpretedFunction::EvaluatePartial(const Environment& env) const {
  std::vector<Expression> new_arguments;
  new_arguments.reserve(arguments_.size());
  for (const Expression& arg : arguments_) {
    new_arguments.push_back(arg.EvaluatePartial(env));
  }
  return uninterpreted_function(name_, std::move(new_arguments));
}

Expression ExpressionUninterpretedFunction::Substitute(const Substitution& s) const {
  std::vector<Expression> new_arguments;
  new_arguments.reserve(arguments_.size());
  for (const Expression& arg : arguments_) {
    new_arguments.push_back(arg.Substitute(s));
  }
  return uninterpreted_function(name_, std::move(new_arguments));
}

Expression ExpressionUninterpretedFunction::Differentiate(const Variable& x) const {
  if (variables().include(x)) {
    // This uninterpreted function does have `x` as an argument, but we don't
    // have sufficient information to differentiate it with respect to `x`.
    std::ostringstream oss;
    oss << "Uninterpreted-function expression ";
    Display(oss);
    oss << " is not differentiable with respect to " << x << ".";
    throw std::runtime_error(oss.str());
  } else {
    // `x` is free in this uninterpreted function.
    return Expression::Zero();
  }
}

std::ostream& ExpressionUninterpretedFunction::Display(std::ostream& os) const {
  os << name_ << "(";
  if (!arguments_.empty()) {
    auto it = arguments_.begin();
    os << *(it++);
    for (; it != arguments_.end(); ++it) {
      os << ", " << *it;
    }
  }
  return os << ")";
}

bool is_variable(const ExpressionCell& c) { return c.kind() == ExpressionKind::Var; }
bool is_addition(const ExpressionCell& c) { return c.kind() == ExpressionKind::Add; }
bool is_multiplication(const ExpressionCell& c) { return c.kind() == ExpressionKind::Mul; }
bool is_division(const ExpressionCell& c) { return c.kind() == ExpressionKind::Div; }
bool is_log(const ExpressionCell& c) { return c.kind() == ExpressionKind::Log; }
bool is_abs(const ExpressionCell& c) { return c.kind() == ExpressionKind::Abs; }
bool is_exp(const ExpressionCell& c) { return c.kind() == ExpressionKind::Exp; }
bool is_sqrt(const ExpressionCell& c) { return c.kind() == ExpressionKind::Sqrt; }
bool is_pow(const ExpressionCell& c) { return c.kind() == ExpressionKind::Pow; }
bool is_sin(const ExpressionCell& c) { return c.kind() == ExpressionKind::Sin; }
bool is_cos(const ExpressionCell& c) { return c.kind() == ExpressionKind::Cos; }
bool is_tan(const ExpressionCell& c) { return c.kind() == ExpressionKind::Tan; }
bool is_asin(const ExpressionCell& c) { return c.kind() == ExpressionKind::Asin; }
bool is_acos(const ExpressionCell& c) { return c.kind() == ExpressionKind::Acos; }
bool is_atan(const ExpressionCell& c) { return c.kind() == ExpressionKind::Atan; }
bool is_atan2(const ExpressionCell& c) { return c.kind() == ExpressionKind::Atan2; }
bool is_sinh(const ExpressionCell& c) { return c.kind() == ExpressionKind::Sinh; }
bool is_cosh(const ExpressionCell& c) { return c.kind() == ExpressionKind::Cosh; }
bool is_tanh(const ExpressionCell& c) { return c.kind() == ExpressionKind::Tanh; }
bool is_min(const ExpressionCell& c) { return c.kind() == ExpressionKind::MIN; }
bool is_max(const ExpressionCell& c) { return c.kind() == ExpressionKind::MAX; }
bool is_ceil(const ExpressionCell& c) { return c.kind() == ExpressionKind::Ceil; }
bool is_floor(const ExpressionCell& c) { return c.kind() == ExpressionKind::Floor; }
bool is_if_then_else(const ExpressionCell& c) { return c.kind() == ExpressionKind::IfThenElse; }
bool is_uninterpreted_function(const ExpressionCell& c) { return c.kind() == ExpressionKind::UninterpretedFunction; }

template <ExpressionCellKind Kind>
const Kind& to(const Expression& e) {
  SMATS_ASSERT(e.kind() == ExpressionKind::Var);
  return static_cast<const Kind&>(e.cell());
}

const ExpressionVar& to_variable(const Expression& e) {
  SMATS_ASSERT(is_variable(e));
  return static_cast<const ExpressionVar&>(e.cell());
}

ExpressionVar& to_variable(Expression* const e) {
  SMATS_ASSERT(e && is_variable(*e));
  return static_cast<ExpressionVar&>(e->mutable_cell());
}

bool is_unary(const ExpressionCell& cell) {
  return (is_log(cell) || is_abs(cell) || is_exp(cell) || is_sqrt(cell) || is_sin(cell) || is_cos(cell) ||
          is_tan(cell) || is_asin(cell) || is_acos(cell) || is_atan(cell) || is_sinh(cell) || is_cosh(cell) ||
          is_tanh(cell) || is_ceil(cell) || is_floor(cell));
}

const UnaryExpressionCell& to_unary(const Expression& e) {
  SMATS_ASSERT(is_unary(e.cell()));
  return static_cast<const UnaryExpressionCell&>(e.cell());
}

UnaryExpressionCell& to_unary(Expression* const e) {
  SMATS_ASSERT(e && is_unary(e->cell()));
  return static_cast<UnaryExpressionCell&>(e->mutable_cell());
}

bool is_binary(const ExpressionCell& cell) {
  return (is_division(cell) || is_pow(cell) || is_atan2(cell) || is_min(cell) || is_max(cell));
}

const BinaryExpressionCell& to_binary(const Expression& e) {
  SMATS_ASSERT(is_binary(e.cell()));
  return static_cast<const BinaryExpressionCell&>(e.cell());
}

BinaryExpressionCell& to_binary(Expression* const e) {
  SMATS_ASSERT(e && is_binary(e->cell()));
  return static_cast<BinaryExpressionCell&>(e->mutable_cell());
}

const ExpressionAdd& to_addition(const Expression& e) {
  SMATS_ASSERT(is_addition(e));
  return static_cast<const ExpressionAdd&>(e.cell());
}

ExpressionAdd& to_addition(Expression* const e) {
  SMATS_ASSERT(e && is_addition(*e));
  return static_cast<ExpressionAdd&>(e->mutable_cell());
}

const ExpressionMul& to_multiplication(const Expression& e) {
  SMATS_ASSERT(is_multiplication(e));
  return static_cast<const ExpressionMul&>(e.cell());
}

ExpressionMul& to_multiplication(Expression* const e) {
  SMATS_ASSERT(e && is_multiplication(*e));
  return static_cast<ExpressionMul&>(e->mutable_cell());
}

const ExpressionDiv& to_division(const Expression& e) {
  SMATS_ASSERT(is_division(e));
  return static_cast<const ExpressionDiv&>(e.cell());
}

ExpressionDiv& to_division(Expression* const e) {
  SMATS_ASSERT(e && is_division(*e));
  return static_cast<ExpressionDiv&>(e->mutable_cell());
}

const ExpressionLog& to_log(const Expression& e) {
  SMATS_ASSERT(is_log(e));
  return static_cast<const ExpressionLog&>(e.cell());
}

ExpressionLog& to_log(Expression* const e) {
  SMATS_ASSERT(e && is_log(*e));
  return static_cast<ExpressionLog&>(e->mutable_cell());
}

const ExpressionAbs& to_abs(const Expression& e) {
  SMATS_ASSERT(is_abs(e));
  return static_cast<const ExpressionAbs&>(e.cell());
}

ExpressionAbs& to_abs(Expression* const e) {
  SMATS_ASSERT(e && is_abs(*e));
  return static_cast<ExpressionAbs&>(e->mutable_cell());
}

const ExpressionExp& to_exp(const Expression& e) {
  SMATS_ASSERT(is_exp(e));
  return static_cast<const ExpressionExp&>(e.cell());
}

ExpressionExp& to_exp(Expression* const e) {
  SMATS_ASSERT(e && is_exp(*e));
  return static_cast<ExpressionExp&>(e->mutable_cell());
}

const ExpressionSqrt& to_sqrt(const Expression& e) {
  SMATS_ASSERT(is_sqrt(e));
  return static_cast<const ExpressionSqrt&>(e.cell());
}

ExpressionSqrt& to_sqrt(Expression* const e) {
  SMATS_ASSERT(e && is_sqrt(*e));
  return static_cast<ExpressionSqrt&>(e->mutable_cell());
}

const ExpressionPow& to_pow(const Expression& e) {
  SMATS_ASSERT(is_pow(e));
  return static_cast<const ExpressionPow&>(e.cell());
}

ExpressionPow& to_pow(Expression* const e) {
  SMATS_ASSERT(e && is_pow(*e));
  return static_cast<ExpressionPow&>(e->mutable_cell());
}

const ExpressionSin& to_sin(const Expression& e) {
  SMATS_ASSERT(is_sin(e));
  return static_cast<const ExpressionSin&>(e.cell());
}

ExpressionSin& to_sin(Expression* const e) {
  SMATS_ASSERT(e && is_sin(*e));
  return static_cast<ExpressionSin&>(e->mutable_cell());
}

const ExpressionCos& to_cos(const Expression& e) {
  SMATS_ASSERT(is_cos(e));
  return static_cast<const ExpressionCos&>(e.cell());
}

ExpressionCos& to_cos(Expression* const e) {
  SMATS_ASSERT(e && is_cos(*e));
  return static_cast<ExpressionCos&>(e->mutable_cell());
}

const ExpressionTan& to_tan(const Expression& e) {
  SMATS_ASSERT(is_tan(e));
  return static_cast<const ExpressionTan&>(e.cell());
}

ExpressionTan& to_tan(Expression* const e) {
  SMATS_ASSERT(e && is_tan(*e));
  return static_cast<ExpressionTan&>(e->mutable_cell());
}

const ExpressionAsin& to_asin(const Expression& e) {
  SMATS_ASSERT(is_asin(e));
  return static_cast<const ExpressionAsin&>(e.cell());
}

ExpressionAsin& to_asin(Expression* const e) {
  SMATS_ASSERT(e && is_asin(*e));
  return static_cast<ExpressionAsin&>(e->mutable_cell());
}

const ExpressionAcos& to_acos(const Expression& e) {
  SMATS_ASSERT(is_acos(e));
  return static_cast<const ExpressionAcos&>(e.cell());
}

ExpressionAcos& to_acos(Expression* const e) {
  SMATS_ASSERT(e && is_acos(*e));
  return static_cast<ExpressionAcos&>(e->mutable_cell());
}

const ExpressionAtan& to_atan(const Expression& e) {
  SMATS_ASSERT(is_atan(e));
  return static_cast<const ExpressionAtan&>(e.cell());
}

ExpressionAtan& to_atan(Expression* const e) {
  SMATS_ASSERT(e && is_atan(*e));
  return static_cast<ExpressionAtan&>(e->mutable_cell());
}

const ExpressionAtan2& to_atan2(const Expression& e) {
  SMATS_ASSERT(is_atan2(e));
  return static_cast<const ExpressionAtan2&>(e.cell());
}

ExpressionAtan2& to_atan2(Expression* const e) {
  SMATS_ASSERT(e && is_atan2(*e));
  return static_cast<ExpressionAtan2&>(e->mutable_cell());
}

const ExpressionSinh& to_sinh(const Expression& e) {
  SMATS_ASSERT(is_sinh(e));
  return static_cast<const ExpressionSinh&>(e.cell());
}

ExpressionSinh& to_sinh(Expression* const e) {
  SMATS_ASSERT(e && is_sinh(*e));
  return static_cast<ExpressionSinh&>(e->mutable_cell());
}

const ExpressionCosh& to_cosh(const Expression& e) {
  SMATS_ASSERT(is_cosh(e));
  return static_cast<const ExpressionCosh&>(e.cell());
}

ExpressionCosh& to_cosh(Expression* const e) {
  SMATS_ASSERT(e && is_cosh(*e));
  return static_cast<ExpressionCosh&>(e->mutable_cell());
}

const ExpressionTanh& to_tanh(const Expression& e) {
  SMATS_ASSERT(is_tanh(e));
  return static_cast<const ExpressionTanh&>(e.cell());
}

ExpressionTanh& to_tanh(Expression* const e) {
  SMATS_ASSERT(e && is_tanh(*e));
  return static_cast<ExpressionTanh&>(e->mutable_cell());
}

const ExpressionMin& to_min(const Expression& e) {
  SMATS_ASSERT(is_min(e));
  return static_cast<const ExpressionMin&>(e.cell());
}

ExpressionMin& to_min(Expression* const e) {
  SMATS_ASSERT(e && is_min(*e));
  return static_cast<ExpressionMin&>(e->mutable_cell());
}

const ExpressionMax& to_max(const Expression& e) {
  SMATS_ASSERT(is_max(e));
  return static_cast<const ExpressionMax&>(e.cell());
}

ExpressionMax& to_max(Expression* const e) {
  SMATS_ASSERT(e && is_max(*e));
  return static_cast<ExpressionMax&>(e->mutable_cell());
}

const ExpressionCeiling& to_ceil(const Expression& e) {
  SMATS_ASSERT(is_ceil(e));
  return static_cast<const ExpressionCeiling&>(e.cell());
}

ExpressionCeiling& to_ceil(Expression* const e) {
  SMATS_ASSERT(e && is_ceil(*e));
  return static_cast<ExpressionCeiling&>(e->mutable_cell());
}

const ExpressionFloor& to_floor(const Expression& e) {
  SMATS_ASSERT(is_floor(e));
  return static_cast<const ExpressionFloor&>(e.cell());
}

ExpressionFloor& to_floor(Expression* const e) {
  SMATS_ASSERT(e && is_floor(*e));
  return static_cast<ExpressionFloor&>(e->mutable_cell());
}

const ExpressionIfThenElse& to_if_then_else(const Expression& e) {
  SMATS_ASSERT(is_if_then_else(e));
  return static_cast<const ExpressionIfThenElse&>(e.cell());
}

ExpressionIfThenElse& to_if_then_else(Expression* const e) {
  SMATS_ASSERT(e && is_if_then_else(*e));
  return static_cast<ExpressionIfThenElse&>(e->mutable_cell());
}

const ExpressionUninterpretedFunction& to_uninterpreted_function(const Expression& e) {
  SMATS_ASSERT(is_uninterpreted_function(e));
  return static_cast<const ExpressionUninterpretedFunction&>(e.cell());
}

ExpressionUninterpretedFunction& to_uninterpreted_function(Expression* const e) {
  SMATS_ASSERT(e && is_uninterpreted_function(*e));
  return static_cast<ExpressionUninterpretedFunction&>(e->mutable_cell());
}

#endif

}  // namespace smats
