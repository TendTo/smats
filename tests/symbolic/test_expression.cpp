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
class TestExpression : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  const Variable z_{"z"};
};

using TestParams = ::testing::Types<int, long, float, double>;
TYPED_TEST_SUITE(TestExpression, TestParams);

TYPED_TEST(TestExpression, ConstantZeroConstructor) {
  Expression<TypeParam> e{0};
  EXPECT_TRUE(e.is_constant());
  EXPECT_FALSE(e.is_variable());
  EXPECT_TRUE(e.is_constant(0));
  EXPECT_FALSE(e.is_constant(1));
  EXPECT_EQ(e.use_count(), 2);
}

TYPED_TEST(TestExpression, ConstantOneConstructor) {
  Expression<TypeParam> e{1};
  EXPECT_TRUE(e.is_constant());
  EXPECT_FALSE(e.is_variable());
  EXPECT_FALSE(e.is_constant(0));
  EXPECT_TRUE(e.is_constant(1));
  EXPECT_EQ(e.use_count(), 2);
}

TYPED_TEST(TestExpression, ConstantConstructor) {
  Expression<TypeParam> e{17};
  EXPECT_TRUE(e.is_constant());
  EXPECT_FALSE(e.is_variable());
  EXPECT_FALSE(e.is_constant(0));
  EXPECT_FALSE(e.is_constant(1));
  EXPECT_TRUE(e.is_constant(17));
  EXPECT_EQ(e.use_count(), 1);
}

TYPED_TEST(TestExpression, VariableConstructor) {
  Expression<TypeParam> x{this->x_};
  EXPECT_FALSE(x.is_constant());
  EXPECT_TRUE(x.is_variable());
  EXPECT_FALSE(x.is_variable(this->y_));
  EXPECT_TRUE(x.is_variable(this->x_));
}
