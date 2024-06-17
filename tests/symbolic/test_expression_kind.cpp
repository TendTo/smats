/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "smats/symbolic/expression_kind.h"

using smats::ExpressionKind;

class TestExpressionKind : public ::testing::TestWithParam<std::pair<ExpressionKind, std::string>> {};

INSTANTIATE_TEST_SUITE_P(
    TestExpressionKind, TestExpressionKind,
    ::testing::Values(std::make_pair(ExpressionKind::Constant, "Constant"), std::make_pair(ExpressionKind::Var, "Var"),
                      std::make_pair(ExpressionKind::Add, "Add"), std::make_pair(ExpressionKind::Mul, "Mul"),
                      std::make_pair(ExpressionKind::Div, "Div"), std::make_pair(ExpressionKind::Pow, "Pow"),
                      std::make_pair(ExpressionKind::Sin, "Sin"), std::make_pair(ExpressionKind::Cos, "Cos"),
                      std::make_pair(ExpressionKind::Tan, "Tan"), std::make_pair(ExpressionKind::Asin, "Asin"),
                      std::make_pair(ExpressionKind::Acos, "Acos"), std::make_pair(ExpressionKind::Atan, "Atan"),
                      std::make_pair(ExpressionKind::Atan2, "Atan2"), std::make_pair(ExpressionKind::Sinh, "Sinh"),
                      std::make_pair(ExpressionKind::Cosh, "Cosh"), std::make_pair(ExpressionKind::Tanh, "Tanh"),
                      std::make_pair(ExpressionKind::Log, "Log"), std::make_pair(ExpressionKind::Exp, "Exp"),
                      std::make_pair(ExpressionKind::Sqrt, "Sqrt"), std::make_pair(ExpressionKind::Abs, "Abs"),
                      std::make_pair(ExpressionKind::Floor, "Floor"), std::make_pair(ExpressionKind::Ceil, "Ceil"),
                      std::make_pair(ExpressionKind::Min, "Min"), std::make_pair(ExpressionKind::Max, "Max"),
                      std::make_pair(ExpressionKind::IfThenElse, "IfThenElse"),
                      std::make_pair(ExpressionKind::NaN, "NaN"),
                      std::make_pair(ExpressionKind::UninterpretedFunction, "UninterpretedFunction")));

TEST_P(TestExpressionKind, ToString) {
  const auto [kind, expected] = GetParam();
  EXPECT_EQ((std::ostringstream{} << kind).str(), expected);
}