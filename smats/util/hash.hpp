/**
 * @file Variable.cpp
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @copyright 2019 Drake (https://drake.mit.edu)
 * @licence Apache-2.0 license
 * @brief Hash functions
 *
 * Dlinear implements the hash_append pattern as described by N3980, based on Drake's implementation.
 * For more information, refer to [N3980](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html).
 *
 * To provide hash_append support within a class, the class must implement a `hash_append` function.
 * The function appends every hash-relevant member field into the hasher.
 * @code
 * class MyValue {
 *   public:
 *    ...
 *     *  Implements the @ref hash_append concept.
 *    template <class HashAlgorithm>
 *    friend void hash_append(
 *        HashAlgorithm& hasher, const MyValue& item) noexcept {
 *      using smats::hash_append;
 *      hash_append(hasher, item.my_data_);
 *    }
 *    ...
 *   private:
 *    std::string my_data_;
 *  };
 * @endcode
 *
 * Checklist for reviewing a `hash_append` implementation:
 * - The function cites `@ref hash_append` in its Doxygen comment.
 * - The function is marked `noexcept`.
 *
 * To use hashable types, Drake types may be used in unordered collections:
 * @code
 * std::unordered_set<MyValue, smats::DefaultHash> foo;
 * @endcode
 *
 * Some types may also choose to specialize `std::hash<MyValue>` to use `DefaultHash`,
 * so that the second template argument to `std::unordered_set` can be omitted.
 * For example @ref smats::symbolic::Expression header states:
 * @code
 * namespace std {
 * struct hash<smats::symbolic::Expression> : public smats::DefaultHash {};
 * }  // namespace std
 * @endcode
 * so that users are able to simply write:
 * @code
 * std::unordered_set<smats::symbolic::Expression> foo;
 * @endcode
 */
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "smats/util/concepts.h"
#include "smats/util/exception.h"

namespace smats {

/**
 * Hash append implementation for an object that implements the @ref Hashable concept.
 * @tparam InvocableHashAlgorithm HashAlgorithm type of the hash algorithm to use
 * @tparam Hashable hashable type
 * @param hasher hash algorithm
 * @param hashable object to hash
 */
template <InvocableHashAlgorithm HashAlgorithm, Hashable<HashAlgorithm> T>
void hash_append(HashAlgorithm& hasher, const T& hashable) noexcept {
  hashable.hash(hasher);
}

/**
 * Hash append implementation for integral types
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam std::integral integral type
 * @param hasher hash algorithm
 * @param item integral to hash
 */
void hash_append(InvocableHashAlgorithm auto& hasher, const std::integral auto& item) noexcept {
  hasher(std::addressof(item), sizeof(item));
}

/**
 * Hash append implementation for pointer types
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam T any type
 * @param hasher hash algorithm
 * @param item pointer to hash
 */
template <class T>
void hash_append(InvocableHashAlgorithm auto& hasher, const T* item) noexcept {
  using smats::hash_append;
  hash_append(hasher, reinterpret_cast<std::uintptr_t>(item));
};

/**
 * Hash append implementation for enum types
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam IsEnum enum type
 * @param hasher hash algorithm
 * @param item enum to hash
 */
void hash_append(InvocableHashAlgorithm auto& hasher, const IsEnum auto& item) noexcept {
  using smats::hash_append;
  hasher(std::addressof(item), sizeof(item));
}

/**
 * Hash append implementation for floating point types
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam std::floating_point T floating point type
 * @param hasher hash algorithm
 * @param item floating point to hash
 */
template <std::floating_point T>
void hash_append(InvocableHashAlgorithm auto& hasher, const T& item) noexcept {
  // Hashing a NaN makes no sense, since they cannot compare as equal.
  SMATS_ASSERT(!std::isnan(item), "NaN is not a valid hash key.");
  // +0.0 and -0.0 are equal, so must hash identically.
  if (item == 0.0) {
    const T zero{0.0};
    hasher(std::addressof(zero), sizeof(zero));
  } else {
    hasher(std::addressof(item), sizeof(item));
  }
}

/**
 * Hash append implementation for std::string
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam Traits string traits
 * @tparam Allocator string allocator
 * @param hasher hash algorithm
 * @param item string to hash
 */
template <class Traits, class Allocator>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::basic_string<char, Traits, Allocator>& item) noexcept {
  hasher(item.data(), item.size());
  // All collection types must send their size, after their contents.
  // See the #hash_append_vector anchor in N3980.
  hash_append(hasher, item.size());
}

// Prior to this point, we've defined hashers for primitive types and similar kinds of "singular" types,
// where the template arguments form a bounded set.
//
// Following this point, we'll define hashers for types where the template argument can be anything (e.g., collections).
// That means for proper namespace lookups we need to declare them all first, and then define them all second.

/**
 * Hash append implementation for a pair
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam T1 type of the first element
 * @tparam T2 type of the second element
 * @param hasher hash algorithm
 * @param item pair to hash
 */
template <class T1, class T2>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::pair<T1, T2>& item) noexcept;

/**
 * Hash append implementation for std::optional
 * @note std::hash<std::optional<T>> provides the peculiar invariant that the hash of an `optional` bearing a value `v`
 * shall evaluate to the same hash as that of the value `v` itself.
 * Hash operations implemented with this `hash_append` do *not* provide that invariant.
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam T type of the optional
 * @param hasher hash algorithm
 * @param item optional to hash
 */
template <class T>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::optional<T>& item) noexcept;

/**
 * Hash append implementation for std::map
 * @note There is no `hash_append` overload for `std::unordered_map`.
 * See [N3980](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html#unordered) for details.
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam T1 key type
 * @tparam T2 value type
 * @tparam Compare comparison function
 * @tparam Allocator allocator
 * @param hasher hash algorithm
 * @param item map to hash
 */
template <class T1, class T2, class Compare, class Allocator>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::map<T1, T2, Compare, Allocator>& item) noexcept;

/**
 * Hash append implementation for std::set
 * @note There is no `hash_append` overload for `std::unordered_set`.
 * See [N3980](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3980.html#unordered) for details.
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use
 * @tparam Key key type
 * @tparam Compare comparison function
 * @tparam Allocator allocator
 * @param hasher hash algorithm
 * @param item set to hash
 */
template <class Key, class Compare, class Allocator>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::set<Key, Compare, Allocator>& item) noexcept;

// Now the templates can be defined

template <class T1, class T2>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::pair<T1, T2>& item) noexcept {
  using smats::hash_append;
  hash_append(hasher, item.first);
  hash_append(hasher, item.second);
}
template <class T>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::optional<T>& item) noexcept {
  if (item) {
    hash_append(hasher, *item);
  }
  hash_append(hasher, item.has_value());
};

// NOLINTNEXTLINE(runtime/references) Per hash_append convention.
// Provides @ref hash_append for a range, as given by two iterators.
template <class Iter>
void hash_append_range(InvocableHashAlgorithm auto& hasher, Iter begin, Iter end) noexcept {
  using smats::hash_append;
  size_t count{0};
  for (Iter iter = begin; iter != end; ++iter, ++count) {
    hash_append(hasher, *iter);
  }
  // All collection types must send their size, after their contents.
  // See the #hash_append_vector anchor in N3980.
  hash_append(hasher, count);
}
template <class T1, class T2, class Compare, class Allocator>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::map<T1, T2, Compare, Allocator>& item) noexcept {
  return hash_append_range(hasher, item.begin(), item.end());
};
template <class Key, class Compare, class Allocator>
void hash_append(InvocableHashAlgorithm auto& hasher, const std::set<Key, Compare, Allocator>& item) noexcept {
  return hash_append_range(hasher, item.begin(), item.end());
};

/**
 * Hash functor
 *
 * It behaves like std::hash, producing a hash value for a given item of type T, applying @ref hash_append to it.
 * How the has is calculated depends on the @p HashAlgorithm.
 * @tparam InvocableHashAlgorithm type of the hash algorithm to use.
 */
template <class HashAlgorithm>
struct uhash {
  using result_type = typename HashAlgorithm::result_type;

  template <class T>
  result_type operator()(const T& item) const noexcept {
    HashAlgorithm hasher;
    using smats::hash_append;
    hash_append(hasher, item);
    return static_cast<result_type>(hasher);
  }
};

namespace hash {
/**
 * FNV-1a hash algorithm.
 *
 * This is a 64-bit implementation of the FNV-1a hash algorithm used by @ref hash_append.
 * See [FNV-1a](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function) for details.
 */
class FNV1a {
 public:
  using result_type = size_t;

  /**
   * Append [data, data + length) bytes into this hash.
   * @param data data to hash
   * @param length length of the data
   */
  void operator()(const void* data, const size_t length) noexcept {
    const auto* const begin = static_cast<const uint8_t*>(data);
    const uint8_t* const end = begin + length;
    for (const uint8_t* iter = begin; iter < end; ++iter) {
      hash_ = (hash_ ^ *iter) * k_fnv_prime;
    }
  }

  /**
   * Append a byte into this hash.
   * @param byte byte to hash
   */
  constexpr void add_byte(const uint8_t byte) noexcept { hash_ = (hash_ ^ byte) * k_fnv_prime; }

  /** @getter{hash value, FNV1a algorithm} */
  explicit constexpr operator size_t() const noexcept { return hash_; }

 private:
  static_assert(sizeof(result_type) == (64 / 8), "We require a 64-bit size_t");
  result_type hash_{0xcbf29ce484222325u};                ///< FNV-1a offset basis
  static constexpr size_t k_fnv_prime = 1099511628211u;  ///< FNV-1a prime
};
}  // namespace hash

using DefaultHashAlgorithm = hash::FNV1a;                  ///< The default hashing algorithm. The return type is size_t
using DefaultHash = smats::uhash<DefaultHashAlgorithm>;  ///< The default hashing functor. It behaves like std::hash

/**
 * A simple struct that wraps a specific hash algorithm into a std::function.
 * It is useful for passing a concrete HashAlgorithm implementation through into non-templated code.
 * @see ExpressionCell::hash
 */
struct DelegatingHasher {
  using Func = std::function<void(const void*, size_t)>;
  using result_type = typename DefaultHashAlgorithm::result_type;  ///< Necessary to be accepted by hash_append

  /**
   * Construct a new DelegatingHasher object.
   * @pre The function must be non-empty.
   * @param func hash algorithm implementation
   */
  explicit DelegatingHasher(Func func) : func_(std::move(func)) {
    if (!static_cast<bool>(func_)) SMATS_RUNTIME_ERROR("The function must be non-empty");
  }
  /**
   * Delegation of the hash algorithm to @ref func_ .
   * @param data data to hash
   * @param length length of the data
   */
  void operator()(const void* data, size_t length) noexcept { func_(data, length); }
  /**
   * Placeholder for the return type.
   *
   * It is necessary for this struct to correctly implement the @ref Hashable concept.
   * @warning This function should never be called.
   */
  operator result_type() noexcept { SMATS_UNREACHABLE(); }

 private:
  const Func func_;  ///< Concrete hash algorithm implementation
};

}  // namespace smats
