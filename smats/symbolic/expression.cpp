/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Expression class.
 */
#include "smats/symbolic/expression.h"

#include "smats/symbolic/expression_cell.h"
#include "smats/util/exception.h"

namespace smats {

template <class T>
Expression<T>::Expression(const T& value) : cell_{ExpressionConstant<T>::New(value)} {}

template <class T>
Expression<T>::Expression(const Variable& var) : cell_{ExpressionVar<T>::New(var)} {}

template <class T>
ExpressionKind Expression<T>::kind() const {
  return cell_->kind();
}

template <class T>
bool Expression<T>::equal_to(const Expression<T>& e) const {
  return false;
}

template <class T>
bool Expression<T>::is_constant() const {
  return cell_->kind() == ExpressionKind::Constant;
}

template class Expression<int>;
template class Expression<long>;
template class Expression<float>;
template class Expression<double>;

}  // namespace smats
