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

class TestExpressionConstant : public ::testing::Test {
 protected:
  const ExpressionConstant zero{0.0};
  const ExpressionConstant one{1.0};
  const ExpressionConstant two{2.0};
  const ExpressionConstant three{3.0};
};

TEST_F(TestExpressionConstant, DummyConstructor) {
  const ExpressionConstant<int> zero;
  EXPECT_EQ(zero.value(), 0);
}
