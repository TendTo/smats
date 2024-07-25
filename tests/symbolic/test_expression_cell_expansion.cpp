/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include <cmath>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "smats/symbolic/expression_cell.h"

using smats::Environment;
using smats::Expression;
using smats::Variable;

#define EXPECT_EXPAND_PRESERVE_EVALUATION(e, eps) EXPECT_TRUE(this->check_expand_preserve_evaluate(e, eps))
#define EXPECT_ALREADY_EXPANDED(e) EXPECT_TRUE(this->check_already_expanded(e))
#define EXPECT_UNCHANGED_EXPAND(e) EXPECT_TRUE(this->check_unchanged_expand(e))
#define EXPECT_EXPAND_IDEMPOTENT(e) EXPECT_TRUE(this->check_expand_idempotent(e))

template <class T>
class TestExpressionCellExpansion : public ::testing::Test {
 protected:
  const Variable var_x_{"x"};
  const Variable var_y_{"y"};
  const Variable var_z_{"z"};

  const Expression<T> x_{var_x_};
  const Expression<T> y_{var_y_};
  const Expression<T> z_{var_z_};

  std::vector<Environment<T>> envs_;

  TestExpressionCellExpansion() {
    // Set up environments (envs_).
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(1.7)}, {var_y_, static_cast<T>(2)}, {var_z_, static_cast<T>(2.3)}});  // + + +
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(-0.3)}, {var_y_, static_cast<T>(1)}, {var_z_, static_cast<T>(0.2)}});  // - + +
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(1.4)}, {var_y_, static_cast<T>(-2)}, {var_z_, static_cast<T>(3.1)}});  // + - +
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(2.2)}, {var_y_, static_cast<T>(4)}, {var_z_, static_cast<T>(-2.3)}});  // + + -
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(-4.7)}, {var_y_, static_cast<T>(-3)}, {var_z_, static_cast<T>(3.4)}});  // - - +
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(3.1)}, {var_y_, static_cast<T>(-3)}, {var_z_, static_cast<T>(-2.5)}});  // + - -
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(-2.8)}, {var_y_, static_cast<T>(2)}, {var_z_, static_cast<T>(-2.6)}});  // - + -
    envs_.push_back(Environment<T>{
        {var_x_, static_cast<T>(-2.2)}, {var_y_, static_cast<T>(-4)}, {var_z_, static_cast<T>(-2.3)}});  // - - -
  }

  // Check if both e and e.Expand() are evaluated to the close-enough (<eps)
  // values under all symbolic environments in envs_.
  bool check_expand_preserve_evaluate(const Expression<T>& e, const T& eps) {
    return std::all_of(envs_.begin(), envs_.end(), [&](const Environment<T>& env) {
      return std::abs(e.evaluate(env) - e.expand().evaluate(env)) < eps;
    });
  }

  // Checks that e.is_expanded() is already true, and that the e == e.Expand()
  // invariant holds.
  bool check_already_expanded(const Expression<T>& e) { return e.is_expanded() && e.equal_to(e.expand()); }

  // Checks that e.is_expanded() is conservatively detected as being false, but
  // that the e == e.Expand() invariant still holds. This means that we could
  // imagine having `e.is_expanded()` be improved to report `true` in this case,
  // in the future if we found it helpful.
  bool check_unchanged_expand(const Expression<T>& e) { return !e.is_expanded() && e.equal_to(e.expand()); }

  // Checks if e.Expand() == e.Expand().Expand().
  bool check_expand_idempotent(const Expression<T>& e) { return e.expand().equal_to(e.expand().expand()); }
};

using TestParams = ::testing::Types<int, long, float, double>;
TYPED_TEST_SUITE(TestExpressionCellExpansion, TestParams);

TYPED_TEST(TestExpressionCellExpansion, ExpressionAlreadyExpandedPolynomial) {
  // The following are all already expanded.
  EXPECT_ALREADY_EXPANDED(0);
  EXPECT_ALREADY_EXPANDED(1);
  EXPECT_ALREADY_EXPANDED(-1);
  EXPECT_ALREADY_EXPANDED(42);
  EXPECT_ALREADY_EXPANDED(-5);
  EXPECT_ALREADY_EXPANDED(this->x_);
  EXPECT_ALREADY_EXPANDED(-this->x_);
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(3) * this->x_);
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(-2) * this->x_);
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(2) * this->x_);
  EXPECT_ALREADY_EXPANDED(this->x_ + this->y_);
  EXPECT_ALREADY_EXPANDED(this->x_ + this->y_);
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(3) * this->x_ * this->y_);    // 3xy
  EXPECT_ALREADY_EXPANDED((this->x_ ^ static_cast<TypeParam>(2)) * this->y_);  // 3x^2y
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(3) * (this->x_ ^ static_cast<TypeParam>(2)) /
                          static_cast<TypeParam>(10) * this->y_);             // 3/10*x^2y
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(-7) + this->x_ + this->y_);  // -7 + x + y
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(1) + static_cast<TypeParam>(3) * this->x_ -
                          static_cast<TypeParam>(4) * this->y_);             // 1 + 3x -4y
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(2) * this->x_ ^ this->y_);  // 2x^y
}

TYPED_TEST(TestExpressionCellExpansion, ExpressionAlreadyExpandedPow) {
  EXPECT_ALREADY_EXPANDED(static_cast<TypeParam>(3) * (static_cast<TypeParam>(3) ^ this->y_));
  EXPECT_ALREADY_EXPANDED(this->x_ ^ this->y_);                    // x^y
  EXPECT_ALREADY_EXPANDED(this->x_ ^ static_cast<TypeParam>(-1));  // x^(-1)

  // The following are all already expanded. They do not yet detect and report
  // `is_expanded() == true` upon construction, but in any case they must not
  // change form when `Expand()` is called.
  EXPECT_UNCHANGED_EXPAND((this->x_ + this->y_) ^ -1);  // (x + y)^(-1)
  EXPECT_UNCHANGED_EXPAND((this->x_ + this->y_) ^
                          (static_cast<TypeParam>(1) / static_cast<TypeParam>(2)));  // (x + y)^(0.5)
  EXPECT_UNCHANGED_EXPAND((this->x_ + this->y_) ^
                          (static_cast<TypeParam>(5) / static_cast<TypeParam>(2)));  // (x + y)^(2.5)
  EXPECT_UNCHANGED_EXPAND((this->x_ + this->y_) ^ (this->x_ - this->y_));            // (x + y)^(x - y)
}

#if 0
TYPED_TEST(TestExpressionCellExpansion, ExpressionExpansion) {
  // test_exprs includes pairs of expression `e` and its expected expansion
  // `expected`. For each pair (e, expected), we check the following:
  //     1. e.Expand() is structurally equal to expected.
  //     2. Evaluate e and e.Expand() under multiple environments to check the
  //        correctness of expansions.
  //     3. A expansion is a fixpoint of Expand() function. That is, a expanded
  //        expression shouldn't be expanded further.

  vector<pair<Expression, Expression>> test_exprs;

  // (2xy²)² = 4x²y⁴
  test_exprs.emplace_back(pow(2 * x_ * y_ * y_, 2), 4 * pow(x_, 2) * pow(y_, 4));

  //   5 * (3 + 2y) + 30 * (7 + x_)
  // = 15 + 10y + 210 + 30x
  // = 225 + 30x + 10y
  test_exprs.emplace_back(5 * (3 + 2 * y_) + 30 * (7 + x_), 225 + 30 * x_ + 10 * y_);

  // (x + 3y) * (2x + 5y) = 2x^2 + 11xy + 15y^2
  test_exprs.emplace_back((x_ + 3 * y_) * (2 * x_ + 5 * y_), 2 * pow(x_, 2) + 11 * x_ * y_ + 15 * pow(y_, 2));

  //   (7 + x) * (5 + y) * (6 + z)
  // = (35 + 5x + 7y + xy) * (6 + z)
  // = (210 + 30x + 42y + 6xy) + (35z + 5xz + 7yz + xyz)
  test_exprs.emplace_back((7 + x_) * (5 + y_) * (6 + z_),
                          210 + 30 * x_ + 42 * y_ + 6 * x_ * y_ + 35 * z_ + 5 * x_ * z_ + 7 * y_ * z_ + x_ * y_ * z_);

  //   (x + 3y) * (2x + 5y) * (x + 3y)
  // = (2x^2 + 11xy + 15y^2) * (x + 3y)
  // = 2x^3 + 11x^2y + 15xy^2
  //        +  6x^2y + 33xy^2 + 45y^3
  // = 2x^3 + 17x^2y + 48xy^2 + 45y^3
  test_exprs.emplace_back((x_ + 3 * y_) * (2 * x_ + 5 * y_) * (x_ + 3 * y_),
                          2 * pow(x_, 3) + 17 * pow(x_, 2) * y_ + 48 * x_ * pow(y_, 2) + 45 * pow(y_, 3));

  //   pow((x + y)^2 + 1, (x - y)^2)
  // = pow(x^2 + 2xy + y^2 + 1, x^2 -2xy + y^2)
  // Expand the base and exponent of pow.
  test_exprs.emplace_back(pow(pow(x_ + y_, 2) + 1, pow(x_ - y_, 2)),
                          pow(pow(x_, 2) + 2 * x_ * y_ + pow(y_, 2) + 1, pow(x_, 2) - 2 * x_ * y_ + pow(y_, 2)));

  //   (x + y + 1)^3
  // = x^3 + 3x^2y +
  //   3x^2 + 3xy^2 + 6xy + 3x +
  //   y^3 + 3y^2 + 3y + 1
  test_exprs.emplace_back(pow(x_ + y_ + 1, 3), pow(x_, 3) + 3 * pow(x_, 2) * y_ + 3 * pow(x_, 2) + 3 * x_ * pow(y_, 2) +
                                                   6 * x_ * y_ + 3 * x_ + pow(y_, 3) + 3 * pow(y_, 2) + 3 * y_ + 1);

  // (x + y + 1)^4
  // = 1 + 4x + 4y + 12xy + 12xy^2 +
  //       4xy^3 + 12x^2y +
  //       6x^2y^2 + 4x^3y +
  //       6x^2 + 4x^3 + x^4 +
  //       6y^2 + 4y^3 + y^4
  test_exprs.emplace_back(pow(x_ + y_ + 1, 4), 1 + 4 * x_ + 4 * y_ + 12 * x_ * y_ + 12 * x_ * pow(y_, 2) +
                                                   4 * x_ * pow(y_, 3) + 12 * pow(x_, 2) * y_ +
                                                   6 * pow(x_, 2) * pow(y_, 2) + 4 * pow(x_, 3) * y_ + 6 * pow(x_, 2) +
                                                   4 * pow(x_, 3) + pow(x_, 4) + 6 * pow(y_, 2) + 4 * pow(y_, 3) +
                                                   pow(y_, 4));

  for (const pair<Expression, Expression>& p : test_exprs) {
    const Expression& e{p.first};
    const Expression expanded{e.Expand()};
    const Expression& expected{p.second};
    EXPECT_PRED2(ExprEqual, expanded, expected);
    EXPECT_TRUE(expanded.is_expanded());
    EXPECT_TRUE(CheckExpandPreserveEvaluation(e, 1e-8));
    EXPECT_TRUE(CheckExpandIsFixpoint(e));
  }
}

TYPED_TEST(TestExpressionCellExpansion, MathFunctions) {
  // For a math function f(x) and an expression e, we need to have the following
  // property:
  //
  //    f(e).Expand() == f(e.Expand())
  //
  // where '==' is structural equality (Expression::EqualTo).
  using F = function<Expression(const Expression&)>;
  vector<F> contexts;
  // clang-format off
  contexts.push_back([](const Expression& x) { return log(x); });
  contexts.push_back([](const Expression& x) { return abs(x); });
  contexts.push_back([](const Expression& x) { return exp(x); });
  contexts.push_back([](const Expression& x) { return sqrt(x); });
  contexts.push_back([](const Expression& x) { return sin(x); });
  contexts.push_back([](const Expression& x) { return cos(x); });
  contexts.push_back([](const Expression& x) { return tan(x); });
  contexts.push_back([](const Expression& x) { return asin(x); });
  contexts.push_back([](const Expression& x) { return acos(x); });
  contexts.push_back([](const Expression& x) { return atan(x); });
  contexts.push_back([](const Expression& x) { return sinh(x); });
  contexts.push_back([](const Expression& x) { return cosh(x); });
  contexts.push_back([](const Expression& x) { return tanh(x); });
  contexts.push_back([&](const Expression& x) { return min(x, y_); });
  contexts.push_back([&](const Expression& x) { return min(y_, x); });
  contexts.push_back([&](const Expression& x) { return max(x, z_); });
  contexts.push_back([&](const Expression& x) { return max(z_, x); });
  contexts.push_back([](const Expression& x) { return ceil(x); });
  contexts.push_back([](const Expression& x) { return floor(x); });
  contexts.push_back([&](const Expression& x) { return atan2(x, y_); });
  contexts.push_back([&](const Expression& x) { return atan2(y_, x); });
  // clang-format on

  vector<Expression> expressions;
  expressions.push_back(5 * (3 + 2 * y_) + 30 * (7 + x_));
  expressions.push_back((x_ + 3 * y_) * (2 * x_ + 5 * y_));
  expressions.push_back((7 + x_) * (5 + y_) * (6 + z_));
  expressions.push_back((x_ + 3 * y_) * (2 * x_ + 5 * y_) * (x_ + 3 * y_));
  expressions.push_back(pow(pow(x_ + y_, 2) + 1, pow(x_ - y_, 2)));
  expressions.push_back(pow(x_ + y_ + 1, 3));
  expressions.push_back(pow(x_ + y_ + 1, 4));

  for (const F& f : contexts) {
    for (const Expression& e : expressions) {
      const Expression e1{f(e).Expand()};
      const Expression e2{f(e.Expand())};

      EXPECT_PRED2(ExprEqual, e1, e2);
      EXPECT_TRUE(CheckAlreadyExpanded(e1));
      EXPECT_TRUE(CheckAlreadyExpanded(e2));
    }
  }
}

TYPED_TEST(TestExpressionCellExpansion, NaN) {
  // NaN is considered as not expanded so that ExpressionNaN::Expand() is called
  // and throws an exception.
  EXPECT_FALSE(Expression::NaN().is_expanded());
  // NaN should be detected during expansion and throw runtime_error.
  Expression dummy;
  EXPECT_THROW(dummy = Expression::NaN().Expand(), runtime_error);
}

TYPED_TEST(TestExpressionCellExpansion, IfThenElse) {
  const Expression e{if_then_else(x_ > y_, pow(x_ + y_, 2), pow(x_ - y_, 2))};
  Expression dummy;
  EXPECT_THROW(dummy = e.Expand(), runtime_error);
  // An if-then-else expression is considered as not expanded so that
  // ExpressionIfThenElse::Expand() is called and throws an exception.
  EXPECT_FALSE(e.is_expanded());
}

TYPED_TEST(TestExpressionCellExpansion, UninterpretedFunction) {
  const Expression uf1{uninterpreted_function("uf1", {})};
  EXPECT_PRED2(ExprEqual, uf1, uf1.Expand());
  EXPECT_TRUE(uf1.Expand().is_expanded());

  const Expression e1{3 * (x_ + y_)};
  const Expression e2{pow(x_ + y_, 2)};
  const Expression uf2{uninterpreted_function("uf2", {e1, e2})};
  EXPECT_PRED2(ExprNotEqual, uf2, uf2.Expand());
  EXPECT_TRUE(uf2.Expand().is_expanded());
  const Expression uf2_expand_expected{uninterpreted_function("uf2", {e1.Expand(), e2.Expand()})};
  EXPECT_PRED2(ExprEqual, uf2.Expand(), uf2_expand_expected);
}

TYPED_TEST(TestExpressionCellExpansion, DivideByConstant) {
  // (x) / 2 => 0.5 * x
  EXPECT_PRED2(ExprEqual, (x_ / 2).Expand(), 0.5 * x_);

  // 3 / 2 => 3 / 2  (no simplification)
  EXPECT_PRED2(ExprEqual, (Expression(3.0) / 2).Expand(), 3.0 / 2);

  // pow(x, y) / 2 => 0.5 * pow(x, y)
  EXPECT_PRED2(ExprEqual, (pow(x_, y_) / 2).Expand(), 0.5 * pow(x_, y_));

  // (2x) / 2 => x
  EXPECT_PRED2(ExprEqual, ((2 * x_) / 2).Expand(), x_);

  // (10x / 5) / 2 => x
  EXPECT_PRED2(ExprEqual, (10 * x_ / 5 / 2).Expand(), x_);

  // (10x²y³z⁴) / -5 => -2x²y³z⁴
  EXPECT_PRED2(ExprEqual, (10 * pow(x_, 2) * pow(y_, 3) * pow(z_, 4) / -5).Expand(),
               -2 * pow(x_, 2) * pow(y_, 3) * pow(z_, 4));

  // (36xy / 4 / -3) => -3xy
  EXPECT_PRED2(ExprEqual, (36 * x_ * y_ / 4 / -3).Expand(), -3 * x_ * y_);

  std::cerr << (x_ / 2).is_polynomial() << std::endl;

  // (2x + 4xy + 6) / 2 => x + 2xy + 3
  EXPECT_PRED2(ExprEqual, ((2 * x_ + 4 * x_ * y_ + 6) / 2).Expand(), x_ + 2 * x_ * y_ + 3);

  // (4x / 3) * (6y / 2) => 4xy
  EXPECT_PRED2(ExprEqual, ((4 * x_ / 3) * (6 * y_ / 2)).Expand(), 4 * x_ * y_);

  // (6xy / z / 3) => 2xy / z
  EXPECT_PRED2(ExprEqual, (6 * x_ * y_ / z_ / 3).Expand(), 2 * x_ * y_ / z_);

  // (36xy / x / -3) => -12xy / x
  // Note that we do not cancel x out since it can be zero.
  EXPECT_PRED2(ExprEqual, (36 * x_ * y_ / x_ / -3).Expand(), -12 * x_ * y_ / x_);
}

TYPED_TEST(TestExpressionCellExpansion, RepeatedExpandShouldBeNoop) {
  const Expression e{(x_ + y_) * (x_ + y_)};

  // New ExpressionCells are created here.
  const Expression e_expanded{e.Expand()};
  {
    LimitMalloc guard;
    // e_expanded is already expanded, so the following line should not create a
    // new cell and no memory allocation should occur. We use LimitMalloc to
    // check this claim.
    const Expression e_expanded_expanded = e_expanded.Expand();
    EXPECT_PRED2(ExprEqual, e_expanded, e_expanded_expanded);
  }
}

TYPED_TEST(TestExpressionCellExpansion, ExpandMultiplicationsWithDivisions) {
  const Expression e1{((x_ + 1) / y_) * (x_ + 3)};
  const Expression e2{(x_ + 3) * ((x_ + 1) / z_)};
  const Expression e3{((x_ + 1) / (y_ + 6)) * ((x_ + 3) / (z_ + 7))};
  const Expression e4{(x_ + y_ / ((z_ + x_) * (y_ + x_))) * (x_ - y_ / z_) * (x_ * y_ / z_)};

  EXPECT_TRUE(CheckExpandIsFixpoint(e1));
  EXPECT_TRUE(CheckExpandIsFixpoint(e2));
  EXPECT_TRUE(CheckExpandIsFixpoint(e3));
  EXPECT_TRUE(CheckExpandIsFixpoint(e4));

  EXPECT_TRUE(CheckExpandPreserveEvaluation(e1, 1e-8));
  EXPECT_TRUE(CheckExpandPreserveEvaluation(e2, 1e-8));
  EXPECT_TRUE(CheckExpandPreserveEvaluation(e3, 1e-8));
  EXPECT_TRUE(CheckExpandPreserveEvaluation(e4, 1e-8));
}
#endif
