/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * @brief Expression evaluator class.
 *
 * The ExpressionEvaluator is used to evaluate an expression with a given box.
 * The box provides the values of the variables in the expression with intervals.
 */
#pragma once

#include <iosfwd>

#include "smats/symbolic/symbolic.h"
#include "smats/util/Box.h"

namespace smats {

/**
 * Evaluate an expression with a given box.
 *
 * The ExpressionEvaluator is used to evaluate an expression with a given box.
 * The box provides the values of the variables in the expression with intervals.
 */
class ExpressionEvaluator {
 public:
  explicit ExpressionEvaluator(Expression e);

  /// Evaluates the expression with @p box.
  Box::Interval operator()(const Box& box) const;

  [[nodiscard]] const Variables& variables() const { return e_.GetVariables(); }

  [[nodiscard]] const Expression& expression() const { return e_; }

 private:
  [[nodiscard]] Box::Interval Visit(const Expression& e, const Box& box) const;
  static Box::Interval VisitVariable(const Expression& e, const Box& box);
  static Box::Interval VisitConstant(const Expression& e, const Box& box);
  static Box::Interval VisitRealConstant(const Expression& e, const Box& box);
  Box::Interval VisitAddition(const Expression& e, const Box& box) const;
  Box::Interval VisitMultiplication(const Expression& e, const Box& box) const;
  Box::Interval VisitDivision(const Expression& e, const Box& box) const;
  Box::Interval VisitLog(const Expression& e, const Box& box) const;
  Box::Interval VisitAbs(const Expression& e, const Box& box) const;
  Box::Interval VisitExp(const Expression& e, const Box& box) const;
  Box::Interval VisitSqrt(const Expression& e, const Box& box) const;
  Box::Interval VisitPow(const Expression& e, const Box& box) const;

  // Evaluates `pow(e1, e2)` with the @p box.
  Box::Interval VisitPow(const Expression& e1, const Expression& e2, const Box& box) const;
  Box::Interval VisitSin(const Expression& e, const Box& box) const;
  Box::Interval VisitCos(const Expression& e, const Box& box) const;
  Box::Interval VisitTan(const Expression& e, const Box& box) const;
  Box::Interval VisitAsin(const Expression& e, const Box& box) const;
  Box::Interval VisitAcos(const Expression& e, const Box& box) const;
  Box::Interval VisitAtan(const Expression& e, const Box& box) const;
  Box::Interval VisitAtan2(const Expression& e, const Box& box) const;
  Box::Interval VisitSinh(const Expression& e, const Box& box) const;
  Box::Interval VisitCosh(const Expression& e, const Box& box) const;
  Box::Interval VisitTanh(const Expression& e, const Box& box) const;
  Box::Interval VisitMin(const Expression& e, const Box& box) const;
  Box::Interval VisitMax(const Expression& e, const Box& box) const;
  static Box::Interval VisitIfThenElse(const Expression& e, const Box& box);
  static Box::Interval VisitUninterpretedFunction(const Expression& e, const Box& box);

  // Makes VisitExpression a friend of this class so that it can use private
  // operator()s.
  friend Box::Interval drake::symbolic::VisitExpression<Box::Interval>(const ExpressionEvaluator*, const Expression&,
                                                                       const Box&);

  const Expression e_;
};

std::ostream& operator<<(std::ostream& os, const ExpressionEvaluator& expression_evaluator);

}  // namespace smats
