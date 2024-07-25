/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Symbolic utilities.
 *
 * A collection of utilities to work with symbolic expressions and formulas.
 */
#pragma once

namespace smats {

/**
 * Helper function to check if a value @p v is an integer.
 *
 * A value is an integer if it is in the range of [int_min, int_max] and its fractional part is zero.
 * @tparam T type of the value
 * @param v value to check
 * @return true if @p v can be represented as an integer
 * @return false if @p v cannot be represented as an integer
 */
template <class T>
bool is_integer(const T& v);

}  // namespace smats
