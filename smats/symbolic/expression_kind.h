/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @copyright 2019 Drake (https://drake.mit.edu)
 * @licence Apache-2.0 license
 * @brief ExpressionKind enum
 *
 * Kinds of symbolic expressions.
 */
#pragma once

#include <cstdint>
#include <iostream>

#include "smats/util/logging.h"

namespace smats {

/**
 * Kinds of symbolic expressions.
 */
enum class ExpressionKind {
  Constant,               ///< constant (double)
  Var,                    ///< variable
  Add,                    ///< addition (+)
  Mul,                    ///< multiplication (*)
  Div,                    ///< division (/)
  Log,                    ///< logarithms
  Abs,                    ///< absolute value function
  Exp,                    ///< exponentiation
  Sqrt,                   ///< square root
  Pow,                    ///< power function
  Sin,                    ///< sine
  Cos,                    ///< cosine
  Tan,                    ///< tangent
  Asin,                   ///< arcsine
  Acos,                   ///< arccosine
  Atan,                   ///< arctangent
  Atan2,                  ///< arctangent2 (atan2(y,x) = atan(y/x))
  Sinh,                   ///< hyperbolic sine
  Cosh,                   ///< hyperbolic cosine
  Tanh,                   ///< hyperbolic tangent
  Min,                    ///< min
  Max,                    ///< max
  Ceil,                   ///< ceil
  Floor,                  ///< floor
  IfThenElse,             ///< if then else
  NaN,                    ///< NaN
  UninterpretedFunction,  ///< Uninterpreted function
};

inline bool operator<(ExpressionKind k1, ExpressionKind k2) {
  return static_cast<std::uint16_t>(k1) < static_cast<std::uint16_t>(k2);
}

std::ostream &operator<<(std::ostream &os, const ExpressionKind &kind);

}  // namespace smats

OSTREAM_FORMATTER(smats::ExpressionKind);
