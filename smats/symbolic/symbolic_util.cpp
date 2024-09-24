/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "symbolic_util.h"

#include <cmath>
#include <limits>

namespace smats {

template <>
bool is_integer(const int& v) {
  return true;
}
template <>
bool is_integer(const long& v) {
  return std::numeric_limits<int>::lowest() <= v && v <= std::numeric_limits<int>::max();
}
template <>
bool is_integer(const float& v) {
  return modf(v, nullptr) == 0.0;
}
template <>
bool is_integer(const double& v) {
  return modf(v, nullptr) == 0.0;
}

template <>
int pow(const int& base, const int& exponent) {
  return static_cast<int>(std::pow(base, exponent));
}
template <>
long pow(const long& base, const long& exponent) {
  return static_cast<long>(std::pow(base, exponent));
}
template <>
float pow(const float& base, const float& exponent) {
  return std::pow(base, exponent);
}
template <>
double pow(const double& base, const double& exponent) {
  return std::pow(base, exponent);
}

}  // namespace smats