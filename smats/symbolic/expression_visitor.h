/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @copyright 2019 Drake (https://drake.mit.edu)
 * @licence Apache-2.0 license
 * @brief ExpressionVisitor class
 *
 * Base class for expression visitors.
 */
#pragma once

#include "smats/symbolic/expression.h"
#include "smats/symbolic/expression_kind.h"
#include "smats/util/exception.h"
#include "smats/util/visitor.h"

namespace smats {

template <class T, class... Args>
class ExpressionVisitor : public Visitor<Expression<T>> {
 public:
  void visit(const Expression<T> &e, Args... args) override {
    switch (e.kind()) {
      case ExpressionKind::Constant:
        visit_constant(e, args...);
        break;
      case ExpressionKind::Var:
        visit_var(e, args...);
        break;
      case ExpressionKind::Add:
        visit_add(e, args...);
        break;
      case ExpressionKind::Mul:
        visit_mul(e, args...);
        break;
      case ExpressionKind::Div:
        visit_div(e, args...);
        break;
      case ExpressionKind::Log:
        visit_log(e, args...);
        break;
      case ExpressionKind::Abs:
        visit_abs(e, args...);
        break;
      case ExpressionKind::Pow:
        visit_pow(e, args...);
        break;
      default:
        throw SmatsUnreachable("Unknown expression kind");
    }
  }

 private:
  virtual void visit_constant(Expression<T> &e, Args... args) = 0;
  virtual void visit_var(Expression<T> &e, Args... args) = 0;
  virtual void visit_add(Expression<T> &e, Args... args) = 0;
  virtual void visit_mul(Expression<T> &e, Args... args) = 0;
  virtual void visit_div(Expression<T> &e, Args... args) = 0;
  virtual void visit_log(Expression<T> &e, Args... args) = 0;
  virtual void visit_abs(Expression<T> &e, Args... args) = 0;
  virtual void visit_pow(Expression<T> &e, Args... args) = 0;
};

}  // namespace smats
