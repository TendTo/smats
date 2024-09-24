/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * ExpressionCell factory classes
 */
#pragma once

#include <map>
#include <memory>

#include "smats/symbolic/expression.h"
#include "smats/symbolic/expression_cell.h"

namespace smats {

template <class T>
class ExpressionAddFactory {
 public:
  using ExpressionMap = std::map<Expression<T>, T>;

  ExpressionAddFactory() = default;
  ExpressionAddFactory(T constant, ExpressionMap expr_to_coeff_map);
  explicit ExpressionAddFactory(const Expression<T> &e);
  explicit ExpressionAddFactory(const std::shared_ptr<const ExpressionCell<T>> &e);
  explicit ExpressionAddFactory(const ExpressionCell<T> &e);

  ExpressionAddFactory<T> &operator+=(const ExpressionAddFactory<T> &o);
  ExpressionAddFactory<T> &operator+=(const T &o);
  ExpressionAddFactory<T> &operator+=(const Expression<T> &o);
  ExpressionAddFactory<T> &operator+=(const std::shared_ptr<ExpressionCell<T>> &o);
  ExpressionAddFactory<T> &operator+=(const ExpressionCell<T> &o);

  ExpressionAddFactory<T> &add(const ExpressionAddFactory<T> &o);
  ExpressionAddFactory<T> &add(const T &o);
  ExpressionAddFactory<T> &add(const Expression<T> &o);
  ExpressionAddFactory<T> &add(const std::shared_ptr<ExpressionCell<T>> &o);
  ExpressionAddFactory<T> &add(const ExpressionCell<T> &o);

  ExpressionAddFactory<T> &add(const T &constant, const ExpressionMap &expr_to_coeff_map);
  ExpressionAddFactory<T> &add(const T &coeff, const Expression<T> &expr);

  ExpressionAddFactory<T> &negate();

  [[nodiscard]] Expression<T> build() const;
  [[nodiscard]] Expression<T> consume();

 private:
  bool consumed_{false};
  T constant_{0};
  ExpressionMap expr_to_coeff_map_;
};

template <class T>
class ExpressionMulFactory {
 public:
  using ExpressionMap = std::map<Expression<T>, Expression<T>>;

  ExpressionMulFactory(T constant, ExpressionMap expr_to_coeff_map);
  explicit ExpressionMulFactory(const Expression<T> &e);
  explicit ExpressionMulFactory(const std::shared_ptr<const ExpressionCell<T>> &e);
  explicit ExpressionMulFactory(const ExpressionCell<T> &e);

  ExpressionMulFactory<T> &operator*=(const T &o);
  ExpressionMulFactory<T> &operator*=(const Expression<T> &o);
  ExpressionMulFactory<T> &operator*=(const std::shared_ptr<ExpressionCell<T>> &o);
  ExpressionMulFactory<T> &operator*=(const ExpressionCell<T> &o);

  ExpressionMulFactory<T> &multiply(const T &o);
  ExpressionMulFactory<T> &multiply(const Expression<T> &o);
  ExpressionMulFactory<T> &multiply(const std::shared_ptr<ExpressionCell<T>> &o);
  ExpressionMulFactory<T> &multiply(const ExpressionCell<T> &o);
  ExpressionMulFactory<T> &multiply(const T &constant, const ExpressionMap &expr_to_coeff_map);
  ExpressionMulFactory<T> &multiply(const Expression<T> &base, const Expression<T> &exponent);

  ExpressionMulFactory<T> &negate();

  [[nodiscard]] Expression<T> build() const;
  Expression<T> consume();

 private:
  bool consumed_{false};
  T constant_{1};
  ExpressionMap base_to_exponent_map_;
};
}  // namespace smats
