/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include <array>
#include <utility>

#include "smats/symbolic/expression.h"

using smats::Expression;
using smats::Variable;

template <class T>
class TestExpressionFactory : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};
  Expression<T> zero_{0};
};

using TestParams = ::testing::Types<int, long, float, double>;
TYPED_TEST_SUITE(TestExpressionFactory, TestParams);

TYPED_TEST(TestExpressionFactory, ConstantConstructor) {
  Expression<TypeParam> zero{0};
  EXPECT_TRUE(zero.is_constant());
  EXPECT_FALSE(zero.is_variable());
  EXPECT_TRUE(zero.is_constant(0));
  EXPECT_FALSE(zero.is_constant(1));
}

TYPED_TEST(TestExpressionFactory, VariableConstructor) {
  Expression<TypeParam> x{this->x_};
  EXPECT_FALSE(x.is_constant());
  EXPECT_TRUE(x.is_variable());
  EXPECT_FALSE(x.is_variable(this->y_));
  EXPECT_TRUE(x.is_variable(this->x_));
}
