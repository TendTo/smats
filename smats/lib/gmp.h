/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#pragma once

#include <gmp.h>    // IWYU pragma: export
#include <gmpxx.h>  // IWYU pragma: export

#include <cctype>
#include <cstring>
#include <string_view>

#include "smats/util/logging.h"

namespace std {

template <>
struct hash<mpq_class> {
  size_t operator()(const mpq_class &val) const;
};
}  // namespace std

namespace smats::gmp {

/**
 * Calculate the floor of a rational number.
 * @param val The rational number.
 * @return The floor of the rational number.
 */
mpz_class floor(const mpq_class &val);
/**
 * Calculate the ceil of a rational number.
 * @param val The rational number.
 * @return The ceil of the rational number.
 */
mpz_class ceil(const mpq_class &val);

/**
 * Cast a mpq_class to a mpq_t.
 *
 * Important definitions from <gmpxx.h> and <gmp.h> (fair use):
 *
 *   mpq_srcptr mpq_class::get_mpq_t() const { return mp; }
 *   mpq_ptr mpq_class::get_mpq_t() { return mp; }
 *
 *   typedef const __mpq_struct *mpq_srcptr;
 *   typedef __mpq_struct *mpq_ptr;
 *   typedef __mpq_struct mpq_t[1];
 *
 * We can cast mpq_ptr to mpq_t * (or mpq_srcptr to const mpq_t *).
 * This is the same as casting (__mpq_struct *) to (__mpq_struct (*)[1]).
 * It's okay because it converts a pointer to a struct, to a pointer to an
 * array of that struct (which is always okay).
 *
 * We can then dereference the (mpq_t *) to obtain a mpq_t.
 * Because mpq_t is an array type, it is still effectively treated as a pointer
 * in certain contexts (such as when returning it from / passing it into a
 * function).
 * This pointer has the same value as the (mpq_t *).
 *
 * We can then take a reference to the mpq_t.
 * The address of this reference also has the same value as the (mpq_t *).
 *
 * @param cla mpq_class to cast
 * @return mpq_t reference
 */
inline const mpq_t &to_mpq_t(const mpq_class &cla) { return *reinterpret_cast<const mpq_t *>(cla.get_mpq_t()); }

inline mpq_t &to_mpq_t(mpq_class &cla) { return *reinterpret_cast<mpq_t *>(cla.get_mpq_t()); }  // NOLINT

/**
 * Cast a mpq_t to a mpq_class.
 *
 * This works because the internal representation of a mpq_class is exactly
 * the same as that of a mpq_t (and, because we only take a reference, no
 * constructor or destructor is ever called).
 * @param mpq mpq_t to cast
 * @return mpq_class reference
 */
inline const mpq_class &to_mpq_class(const mpq_t &mpq) { return reinterpret_cast<const mpq_class &>(mpq); }

/**
 * Cast a mpq_t to a mpq_class.
 *
 * This works because the internal representation of a mpq_class is exactly
 * the same as that of a mpq_t (and, because we only take a reference, no
 * constructor or destructor is ever called).
 * @param mpq mpq_t to cast
 * @return mpq_class reference
 */
inline mpq_class &to_mpq_class(mpq_t &mpq) { return reinterpret_cast<mpq_class &>(mpq); }  // NOLINT

/**
 * Check if the char is either a digit or a plus/minus sign.
 * @param c The char to check.
 * @return True if the char is a digit or a plus/minus sign.
 * @return False otherwise.
 */
inline bool is_digit_or_sign(char c) { return std::isdigit(c) || c == '+' || c == '-'; }

/**
 * Convert a string to a mpq_class.
 *
 * The number is converted exactly, without any rounding,
 * by interpreting the string as a base-10 rational number.
 * @code
 * string_to_mpq("0") == 0
 * string_to_mpq(".") == 0
 * string_to_mpq("0.") == 0
 * string_to_mpq(".0") == 0
 * string_to_mpq("15") == 15/1
 * string_to_mpq("1.5") == 15/10
 * string_to_mpq("15.") == 15/1
 * string_to_mpq(".15") == 15/100
 * string_to_mpq("15.0") == 15/1
 * string_to_mpq("15.00") == 15/1
 * string_to_mpq("15") == 15/1
 * string_to_mpq("1.5E2") == 15/10 * 10^2
 * string_to_mpq("1.5E-2") == 15/10 * 10^-2
 * string_to_mpq("E+2") == 1/1 * 10^2
 * string_to_mpq("15/6") == 15/6
 * string_to_mpq("0/1010") == 0
 * string_to_mpq("inf") == 1e100
 * string_to_mpq("-inf") == -1e100
 * @endcode
 * @note Only a single leading + or - sign is allowed.
 * @warning If the string is not a valid rational number, the result is undefined.
 * @param str The string to convert.
 * @return The mpq_class instance.
 */
mpq_class string_to_mpq(std::string_view str);

}  // namespace smats::gmp

OSTREAM_FORMATTER(mpq_class)
