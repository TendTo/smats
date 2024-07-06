/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Collection of concepts used in the smats library.
 *
 * Utility file that contains c++20 concepts used in the smats library
 * in order to make the code more readable and to provide better error messages.
 */
#pragma once

#include <concepts>

namespace smats {

/**
 * Check if the type T is an enum
 * @tparam T type to check
 */
template <class T>
concept IsEnum = std::is_enum_v<T>;

/**
 * Check if the type T is any of the types U
 * @tparam T type to check
 * @tparam U any number of types to check against
 */
template <class T, class... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

/**
 * Check if the type T is not any of the types U
 * @tparam T type to check
 * @tparam U any number of types to check against
 */
template <class T, class... U>
concept IsNotAnyOf = !IsAnyOf<T, U...>;

/**
 * Check if the type T supports the arithmetic operations +, -, *, /
 * @tparam T type to check
 */
template <class T>
concept Arithmetic = requires(T a, T b) {
  { a + b } -> std::convertible_to<T>;
  { a - b } -> std::convertible_to<T>;
  { a* b } -> std::convertible_to<T>;
  { a / b } -> std::convertible_to<T>;
};  // NOLINT(readability/braces) per C++ standard concept definition

/**
 * Check if the type T supports the arithmetic operations +, -, *, / and the comparison operators <, >, <=, >=
 * @tparam T type to check
 */
template <class T>
concept Numeric = std::totally_ordered<T> && Arithmetic<T>;

/**
 * Check if the type T supports the same operation one could expect a typical hash algorithm to support:
 * - a `result_type` alias that defines the type of the hash value
 * - @code void operator()(const void*, size_t) noexcept @endcode
 *   applies the hash function to the data and append the result to the hash
 * - @code operator result_type () noexcept @endcode
 *   returns the current hash value when casted to the result type
 * @tparam T type to check
 */
template <class T>
concept InvocableHashAlgorithm = requires(T t, const void* data, size_t length) {
  typename T::result_type;
  { t(data, length) } noexcept -> std::same_as<void>;
  { size_t() } noexcept -> std::same_as<typename T::result_type>;
};

/**
 * Check if the type T is hashable, meaning it is not a simple type
 * and provides a `void hash(InvocableHashAlgorithm hasher) noexcept const` method.
 * @tparam T type to check
 * @tparam U return type of the hash value
 */
template <class T, class U>
concept Hashable = requires(T t, U& hasher) {
  { t.hash(hasher) } noexcept -> std::same_as<void>;
} && InvocableHashAlgorithm<U>;

/**
 * Check if the type T can be used to create a span
 * @tparam T type to check
 */
template <class T>
concept Spannable = std::ranges::contiguous_range<T> && std::ranges::sized_range<T>;

/**
 * Check if the type T can be used to create a span of type U
 * @tparam T type to check
 * @tparam U type of elements in the span
 */
template <class T, class U>
concept SpannableOfType = Spannable<T> && std::same_as<U, std::ranges::range_value_t<T>>;

}  // namespace smats
