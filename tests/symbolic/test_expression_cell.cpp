/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include "smats/symbolic/expression_cell.h"

using smats::ExpressionConstant;
using smats::ExpressionNaN;
using smats::ExpressionVar;

template <class T>
class TestExpressionCell : public ::testing::Test {
 protected:
  const ExpressionConstant<T> zero{0};
  const ExpressionConstant<T> one{1};
  const ExpressionConstant<T> two{2};
  const ExpressionConstant<T> three{3};
};

using TestParams = ::testing::Types<int, long, float, double>;
TYPED_TEST_SUITE(TestExpressionCell, TestParams);

TYPED_TEST(TestExpressionCell, ExpressionConstantDefaultConstructor) {
  const ExpressionConstant<TypeParam> e;
  EXPECT_EQ(e.value(), 0);
}

TYPED_TEST(TestExpressionCell, ExpressionConstanValueConstructor) {
  const ExpressionConstant<TypeParam> e{7};
  EXPECT_EQ(e.value(), 7);
}

TYPED_TEST(TestExpressionCell, ExpressionVarDefaultConstructor) {
  const ExpressionVar<TypeParam> e;
  EXPECT_EQ(e.variable().name(), "");
}
