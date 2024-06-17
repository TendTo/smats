/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @copyright 2019 Drake (https://drake.mit.edu)
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/expression_kind.h"

namespace smats {

std::ostream &operator<<(std::ostream &os, const ExpressionKind &kind) {
  switch (kind) {
    case ExpressionKind::Constant:
      return os << "Constant";
    case ExpressionKind::Var:
      return os << "Var";
    case ExpressionKind::Add:
      return os << "Add";
    case ExpressionKind::Mul:
      return os << "Mul";
    case ExpressionKind::Div:
      return os << "Div";
    case ExpressionKind::Pow:
      return os << "Pow";
    case ExpressionKind::Sin:
      return os << "Sin";
    case ExpressionKind::Cos:
      return os << "Cos";
    case ExpressionKind::Tan:
      return os << "Tan";
    case ExpressionKind::Asin:
      return os << "Asin";
    case ExpressionKind::Acos:
      return os << "Acos";
    case ExpressionKind::Atan:
      return os << "Atan";
    case ExpressionKind::Atan2:
      return os << "Atan2";
    case ExpressionKind::Sinh:
      return os << "Sinh";
    case ExpressionKind::Cosh:
      return os << "Cosh";
    case ExpressionKind::Tanh:
      return os << "Tanh";
    case ExpressionKind::Log:
      return os << "Log";
    case ExpressionKind::Exp:
      return os << "Exp";
    case ExpressionKind::Sqrt:
      return os << "Sqrt";
    case ExpressionKind::Abs:
      return os << "Abs";
    case ExpressionKind::Floor:
      return os << "Floor";
    case ExpressionKind::Ceil:
      return os << "Ceil";
    case ExpressionKind::Min:
      return os << "Min";
    case ExpressionKind::Max:
      return os << "Max";
    case ExpressionKind::IfThenElse:
      return os << "IfThenElse";
    case ExpressionKind::NaN:
      return os << "NaN";
    case ExpressionKind::UninterpretedFunction:
      return os << "UninterpretedFunction";
  }
}

}  // namespace smats
