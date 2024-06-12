/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Contains the definition of some useful macros.
 */
#pragma once

/**
 * Add the declaration of the operator @p op for the type @p lhs_type with parameter type @p rhs_type .
 * @code
 * GENERIC_ARITHMETIC_OPERATOR_DECLARATION(ClassA, + ClassB&)
 * // Becomes
 * ClassA operator+(const ClassB& o);
 * @endcode
 * @param lhs_type class to add the operator to
 * @param op operator to add
 * @param rhs_type parameter type for the operator
 */
#define GENERIC_ARITHMETIC_OPERATOR_DECLARATION(lhs_type, op, rhs_type) lhs_type operator op(const rhs_type o);
/**
 * Add the declaration of the operator @p op for the type @p type with itself as the parameter type.
 * @code
 * ARITHMETIC_OPERATOR_DECLARATION(ClassA&, -)
 * // Becomes
 * ClassA& operator-(const ClassA& o);
 * @endcode
 * @param type class to add the operator to and the parameter type
 * @param op operator to add
 */
#define ARITHMETIC_OPERATOR_DECLARATION(type, op) GENERIC_ARITHMETIC_OPERATOR_DECLARATION(type, op, type)
/**
 * Add the declaration for the operators +=, -=, *=, /= for the type @p lhs_type with parameter type @p rhs_type .
 * @code
 * GENERIC_ARITHMETIC_OPERATORS_DECLARATION(ClassA&, ClassB&)
 * // Becomes
 * ClassA& operator+=(const ClassB& o);
 * ClassA& operator-=(const ClassB& o);
 * ClassA& operator*=(const ClassB& o);
 * ClassA& operator/=(const ClassB& o);
 * @endcode
 * @param lhs_type class to add the operator to
 * @param rhs_type parameter type for the operator
 */
#define GENERIC_ARITHMETIC_OPERATORS_DECLARATION(lhs_type, rhs_type) \
  GENERIC_ARITHMETIC_OPERATOR_DECLARATION(lhs_type, +=, rhs_type)    \
  GENERIC_ARITHMETIC_OPERATOR_DECLARATION(lhs_type, -=, rhs_type)    \
  GENERIC_ARITHMETIC_OPERATOR_DECLARATION(lhs_type, *=, rhs_type)    \
  GENERIC_ARITHMETIC_OPERATOR_DECLARATION(lhs_type, /=, rhs_type)
/**
 * Add the declaration for the operators +=, -=, *=, /= for the type @p type with itself as the parameter type.
 * @code
 * GENERIC_ARITHMETIC_OPERATORS_DECLARATION(ClassA&)
 * // Becomes
 * ClassA& operator+=(const ClassA& o);
 * ClassA& operator-=(const ClassA& o);
 * ClassA& operator*=(const ClassA& o);
 * ClassA& operator/=(const ClassA& o);
 * @endcode
 * @param type class to add the operator to and the parameter type
 */
#define ARITHMETIC_OPERATORS_DECLARATION(type) GENERIC_ARITHMETIC_OPERATORS_DECLARATION(type, type)
/**
 * Add the definition of the operator @p op for the type @p lhs_type with parameter type @p rhs_type .
 * @code
 * GENERIC_ARITHMETIC_OPERATOR_DEFINITION(ClassA, +, ClassB&)
 * // Becomes
 * ClassA operator+(const ClassB& o) const {
 *  ClassA temp(*this);
 *  temp += o;
 *  return temp;
 * }
 * @endcode
 * @param lhs_type class to add the operator to
 * @param op operator to add
 * @param rhs_type parameter type for the operator
 */
#define GENERIC_ARITHMETIC_OPERATOR_DEFINITION(lhs_type, op, rhs_type) \
  lhs_type operator op(const rhs_type o) const {                       \
    lhs_type temp(*this);                                              \
    temp op## = o;                                                     \
    return temp;                                                       \
  }
/**
 * Add the definition of the operator @p op for the type @p type with itself as the parameter type.
 * @code
 * ARITHMETIC_OPERATOR_DEFINITION(ClassA, -)
 * // Becomes
 * ClassA& operator-(const ClassA& o) {
 *  ClassA temp(*this);
 *  temp -= o;
 *  return temp;
 * }
 * @endcode
 * @param type class to add the operator to and the parameter type
 * @param op operator to add
 */
#define ARITHMETIC_OPERATOR_DEFINITION(type, op) GENERIC_ARITHMETIC_OPERATOR_DEFINITION(type, op, type &)
/**
 * Add the definition for the operators +, -, *, / for the type @p lhs_type with parameter type @p rhs_type .
 * @code
 * GENERIC_ARITHMETIC_OPERATORS_DEFINITION(ClassA, ClassB&)
 * // Becomes
 * GENERIC_ARITHMETIC_OPERATOR_DEFINITION(ClassA, +, ClassB&)
 * GENERIC_ARITHMETIC_OPERATOR_DEFINITION(ClassA, -, ClassB&)
 * GENERIC_ARITHMETIC_OPERATOR_DEFINITION(ClassA, *, ClassB&)
 * GENERIC_ARITHMETIC_OPERATOR_DEFINITION(ClassA, /, ClassB&)
 * @endcode
 * @param lhs_type class to add the operator to
 * @param rhs_type parameter type for the operator
 * @see GENERIC_ARITHMETIC_OPERATOR_DEFINITION
 */
#define GENERIC_ARITHMETIC_OPERATORS_DEFINITION(lhs_type, rhs_type) \
  GENERIC_ARITHMETIC_OPERATOR_DEFINITION(lhs_type, +, rhs_type)     \
  GENERIC_ARITHMETIC_OPERATOR_DEFINITION(lhs_type, -, rhs_type)     \
  GENERIC_ARITHMETIC_OPERATOR_DEFINITION(lhs_type, *, rhs_type)     \
  GENERIC_ARITHMETIC_OPERATOR_DEFINITION(lhs_type, /, rhs_type)
/**
 * Add the definition for the operators +, -, *, / for the type @p type with itself as the parameter type.
 * @code
 * ARITHMETIC_OPERATORS_DEFINITION(ClassA)
 * // Becomes
 * ARITHMETIC_OPERATOR_DEFINITION(ClassA, +, ClassA&)
 * ARITHMETIC_OPERATOR_DEFINITION(ClassA, -, ClassA&)
 * ARITHMETIC_OPERATOR_DEFINITION(ClassA, *, ClassA&)
 * ARITHMETIC_OPERATOR_DEFINITION(ClassA, /, ClassA&)
 * @endcode
 * @param type class to add the operator to and the parameter type
 * @see ARITHMETIC_OPERATOR_DEFINITION
 */
#define ARITHMETIC_OPERATORS_DEFINITION(type) GENERIC_ARITHMETIC_OPERATORS_DEFINITION(type, type &)
/**
 * Add the declaration for the operators +=, -=, *=, /=
 * and definition for the operators +, -, *, / for the type @p type with parameter type @p rhs_type
 * @code
 * GENERIC_ARITHMETIC_OPERATORS(ClassA, ClassB&)
 * // Becomes
 * GENERIC_ARITHMETIC_OPERATORS_DECLARATION(ClassA&, ClassB&)
 * GENERIC_ARITHMETIC_OPERATORS_DEFINITION(ClassA, ClassB&)
 * @endcode
 * @param type class to add the operator to and the parameter type
 * @see GENERIC_ARITHMETIC_OPERATORS_DECLARATION
 * @see GENERIC_ARITHMETIC_OPERATORS_DEFINITION
 */
#define GENERIC_ARITHMETIC_OPERATORS(lhs_type, rhs_type)         \
  GENERIC_ARITHMETIC_OPERATORS_DECLARATION(lhs_type &, rhs_type) \
  GENERIC_ARITHMETIC_OPERATORS_DEFINITION(lhs_type, rhs_type)
/**
 * Add the declaration for the operators +=, -=, *=, /=
 * and definition for the operators +, -, *, / for the type @p type with itself as the parameter type
 * @code
 * ARITHMETIC_OPERATORS(ClassA)
 * // Becomes
 * ARITHMETIC_OPERATORS_DECLARATION(ClassA&)
 * ARITHMETIC_OPERATORS_DEFINITION(ClassA)
 * @endcode
 * @param type class to add the operator to and the parameter type
 * @see ARITHMETIC_OPERATORS_DECLARATION
 * @see ARITHMETIC_OPERATORS_DEFINITION
 */
#define ARITHMETIC_OPERATORS(type)         \
  ARITHMETIC_OPERATORS_DECLARATION(type &) \
  ARITHMETIC_OPERATORS_DEFINITION(type)
