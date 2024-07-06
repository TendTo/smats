/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @copyright 2019 Drake (https://drake.mit.edu)
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/expression_kind.h"

#include <map>

#include "smats/util/exception.h"

namespace smats {

std::map<ExpressionKind, std::string> expression_kind_to_string = {
    {ExpressionKind::Constant, "Constant"},
    {ExpressionKind::Var, "Var"},
    {ExpressionKind::Add, "Add"},
    {ExpressionKind::Mul, "Mul"},
    {ExpressionKind::Div, "Div"},
    {ExpressionKind::Pow, "Pow"},
    {ExpressionKind::Sin, "Sin"},
    {ExpressionKind::Cos, "Cos"},
    {ExpressionKind::Tan, "Tan"},
    {ExpressionKind::Asin, "Asin"},
    {ExpressionKind::Acos, "Acos"},
    {ExpressionKind::Atan, "Atan"},
    {ExpressionKind::Atan2, "Atan2"},
    {ExpressionKind::Sinh, "Sinh"},
    {ExpressionKind::Cosh, "Cosh"},
    {ExpressionKind::Tanh, "Tanh"},
    {ExpressionKind::Log, "Log"},
    {ExpressionKind::Exp, "Exp"},
    {ExpressionKind::Sqrt, "Sqrt"},
    {ExpressionKind::Abs, "Abs"},
    {ExpressionKind::Floor, "Floor"},
    {ExpressionKind::Ceil, "Ceil"},
    {ExpressionKind::Min, "Min"},
    {ExpressionKind::Max, "Max"},
    {ExpressionKind::IfThenElse, "IfThenElse"},
    {ExpressionKind::NaN, "NaN"},
    {ExpressionKind::UninterpretedFunction, "UninterpretedFunction"}};

std::ostream &operator<<(std::ostream &os, const ExpressionKind &kind) {
  SMATS_ASSERT(expression_kind_to_string.contains(kind), "Unknown ExpressionKind");
  return os << expression_kind_to_string.at(kind);
}

}  // namespace smats
