/**
 * @author smats
 * @date 19 Aug 2023
 * @copyright 2023 smats
 */
#include "smats/symbolic/symbolic.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>

#include "smats/libs/libgmp.h"
#include "smats/util/exception.h"
#include "smats/util/logging.h"

namespace smats {

FormulaKind operator-(const FormulaKind kind) {
  switch (kind) {
    case FormulaKind::Geq:
      return FormulaKind::Leq;
    case FormulaKind::Gt:
      return FormulaKind::Lt;
    case FormulaKind::Leq:
      return FormulaKind::Geq;
    case FormulaKind::Lt:
      return FormulaKind::Gt;
    default:
      return kind;
  }
}

Formula imply(const Formula &f1, const Formula &f2) { return !f1 || f2; }
Formula imply(const Variable &v, const Formula &f) { return imply(Formula{v}, f); }
Formula imply(const Formula &f, const Variable &v) { return imply(f, Formula{v}); }
Formula imply(const Variable &v1, const Variable &v2) { return imply(Formula{v1}, Formula{v2}); }

Formula iff(const Formula &f1, const Formula &f2) { return imply(f1, f2) && imply(f2, f1); }

Formula iff(const Variable &v, const Formula &f) { return iff(Formula{v}, f); }

Formula iff(const Formula &f, const Variable &v) { return iff(f, Formula{v}); }

Formula iff(const Variable &v1, const Variable &v2) { return iff(Formula{v1}, Formula{v2}); }

std::set<Formula> map(const std::set<Formula> &formulas, const std::function<Formula(const Formula &)> &func) {
  std::set<Formula> result;
  std::transform(formulas.cbegin(), formulas.cend(), std::inserter(result, result.begin()), func);
  return result;
}

bool is_atomic(const Formula &f) {
  switch (f.get_kind()) {
    case FormulaKind::False:
    case FormulaKind::True:
    case FormulaKind::Var:
    case FormulaKind::Eq:
    case FormulaKind::Neq:
    case FormulaKind::Gt:
    case FormulaKind::Geq:
    case FormulaKind::Lt:
    case FormulaKind::Leq:
    case FormulaKind::Forall:
      return true;
    case FormulaKind::And:
    case FormulaKind::Or:
      return false;
    case FormulaKind::Not: {
      const Formula &negated_formula{get_operand(f)};
      return is_variable(negated_formula) || is_relational(negated_formula);
    }
    default:
      SMATS_UNREACHABLE();
  }
}

bool is_clause(const Formula &f) {
  if (is_atomic(f)) {
    return true;
  }
  if (is_negation(f)) {
    return is_atomic(get_operand(f));
  }
  if (is_conjunction(f)) {
    return false;
  }
  if (is_disjunction(f)) {
    const auto &operands = get_operands(f);
    // FIXME: should this also be checking for negated atomic formulas?
    const bool result{
        std::all_of(operands.cbegin(), operands.cend(), [](const Formula &formula) { return is_atomic(formula); })};
    return result;
  }
  SMATS_UNREACHABLE();
}

std::set<Formula> get_clauses(const Formula &f) {
  if (is_conjunction(f)) {
#ifndef NDEBUG
    for (const Formula &clause : get_operands(f)) {
      SMATS_ASSERT(is_clause(clause), "Must be a clause");
    }
#endif
    return get_operands(f);
  } else {
    SMATS_ASSERT(is_clause(f), "Must be a clause");
    return {f};
  }
}

bool is_cnf(const Formula &f) {
  if (is_atomic(f)) {
    return true;
  }
  if (is_disjunction(f)) {
    return is_clause(f);
  }
  if (is_conjunction(f)) {
    const auto &operands = get_operands(f);
    const bool result{
        std::all_of(operands.cbegin(), operands.cend(), [](const Formula &formula) { return is_clause(formula); })};
    return result;
  }
  SMATS_UNREACHABLE();
}

bool HaveIntersection(const Variables &variables1, const Variables &variables2) {
  auto begin1 = variables1.begin();
  auto begin2 = variables2.begin();
  const auto end1 = variables1.end();
  const auto end2 = variables2.end();
  while (begin1 != end1 && begin2 != end2) {
    if (begin1->less(*begin2)) {
      ++begin1;
    } else {
      if (!begin2->less(*begin1)) {
        return true;
      }
      ++begin2;
    }
  }
  return false;
}

namespace {
/// Visitor class which strengthens a formula by delta.
class DeltaStrengthenVisitor {
 public:
  DeltaStrengthenVisitor() = default;
  [[nodiscard]] Formula Strengthen(const Formula &f, const double delta) const {
    if (delta == 0) {
      return f;
    }
    return Visit(f, delta);
  }

 private:
  [[nodiscard]] Expression Visit(const Expression &e, const double delta) const {
    return VisitExpression<Expression>(this, e, delta);
  }
  [[nodiscard]] Expression VisitVariable(const Expression &e, const double) const { return e; }
  [[nodiscard]] Expression VisitConstant(const Expression &e, const double) const { return e; }
  [[nodiscard]] Expression VisitAddition(const Expression &e, const double delta) const {
    Expression ret{get_constant_in_addition(e)};
    for (const auto &p : get_expr_to_coeff_map_in_addition(e)) {
      const Expression &e_i{p.first};
      const mpq_class &coeff{p.second};
      ret += coeff * Visit(e_i, delta);
    }
    return ret;
  }
  [[nodiscard]] Expression VisitMultiplication(const Expression &e, const double delta) const {
    Expression ret{get_constant_in_multiplication(e)};
    for (const auto &p : get_base_to_exponent_map_in_multiplication(e)) {
      const Expression &base{p.first};
      const Expression &exponent{p.second};
      ret *= pow(Visit(base, delta), Visit(exponent, delta));
    }
    return ret;
  }
  [[nodiscard]] Expression VisitDivision(const Expression &e, const double delta) const {
    return Visit(get_first_argument(e), delta) / Visit(get_second_argument(e), delta);
  }
  [[nodiscard]] Expression VisitLog(const Expression &e, const double delta) const {
    return log(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitAbs(const Expression &e, const double delta) const {
    return abs(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitExp(const Expression &e, const double delta) const {
    return exp(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitSqrt(const Expression &e, const double delta) const {
    return sqrt(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitPow(const Expression &e, const double delta) const {
    return pow(Visit(get_first_argument(e), delta), Visit(get_second_argument(e), delta));
  }
  [[nodiscard]] Expression VisitSin(const Expression &e, const double delta) const {
    return sin(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitCos(const Expression &e, const double delta) const {
    return cos(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitTan(const Expression &e, const double delta) const {
    return tan(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitAsin(const Expression &e, const double delta) const {
    return asin(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitAcos(const Expression &e, const double delta) const {
    return acos(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitAtan(const Expression &e, const double delta) const {
    return atan(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitAtan2(const Expression &e, const double delta) const {
    return atan2(Visit(get_first_argument(e), delta), Visit(get_second_argument(e), delta));
  }
  [[nodiscard]] Expression VisitSinh(const Expression &e, const double delta) const {
    return sinh(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitCosh(const Expression &e, const double delta) const {
    return cosh(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitTanh(const Expression &e, const double delta) const {
    return tanh(Visit(get_argument(e), delta));
  }
  [[nodiscard]] Expression VisitMin(const Expression &e, const double delta) const {
    return min(Visit(get_first_argument(e), delta), Visit(get_second_argument(e), delta));
  }
  [[nodiscard]] Expression VisitMax(const Expression &e, const double delta) const {
    return max(Visit(get_first_argument(e), delta), Visit(get_second_argument(e), delta));
  }
  [[nodiscard]] Expression VisitIfThenElse(const Expression &e, const double delta) const {
    return if_then_else(Visit(get_conditional_formula(e), delta), Visit(get_then_expression(e), delta),
                        Visit(get_else_expression(e), delta));
  }
  [[nodiscard]] Expression VisitUninterpretedFunction(const Expression &e, const double) const { return e; }

  [[nodiscard]] Formula Visit(const Formula &f, const double delta) const {
    return VisitFormula<Formula>(this, f, delta);
  }
  [[nodiscard]] Formula VisitFalse(const Formula &f, const double) const { return f; }
  [[nodiscard]] Formula VisitTrue(const Formula &f, const double) const { return f; }
  [[nodiscard]] Formula VisitVariable(const Formula &f, const double) const { return f; }
  [[nodiscard]] Formula VisitEqualTo(const Formula &f, const double delta) const {
    if (delta > 0) {
      SMATS_WARN_FMT("Strengthening {} with {} results in false. However, we return {}.", f, delta, f);
      return f;
    } else {
      //     lhs = rhs
      // -> (lhs >= rhs) ∧ (lhs <= rhs)
      const Expression lhs{Visit(get_lhs_expression(f), delta)};
      const Expression rhs{Visit(get_rhs_expression(f), delta)};
      return VisitGreaterThanOrEqualTo(lhs >= rhs, delta) && VisitLessThanOrEqualTo(lhs <= rhs, delta);
    }
  }
  [[nodiscard]] Formula VisitNotEqualTo(const Formula &f, const double delta) const {
    if (delta > 0) {
      //     lhs ≠ rhs
      // -> (lhs > rhs) ∨ (lhs < rhs)
      const Expression lhs{Visit(get_lhs_expression(f), delta)};
      const Expression rhs{Visit(get_rhs_expression(f), delta)};
      return VisitGreaterThan(lhs > rhs, delta) || VisitLessThan(lhs < rhs, delta);
    } else {
      return Formula::True();
    }
  }
  [[nodiscard]] Formula VisitGreaterThan(const Formula &f, const double delta) const {
    //     lhs > rhs
    //
    // After strengthening, we have:
    //     (lhs > rhs + delta)
    const Expression lhs{Visit(get_lhs_expression(f), delta)};
    const Expression rhs{Visit(get_rhs_expression(f), delta)};
    if (is_variable(rhs)) {
      // We return the following so that possibly we can keep the
      // bounded constraint form (c relop. v) where c is a constant.
      // Keeping this syntactic form is useful because our
      // FilterAssertion relies on that.
      return (lhs - delta > rhs);
    } else {
      return (lhs > rhs + delta);
    }
  }
  [[nodiscard]] Formula VisitGreaterThanOrEqualTo(const Formula &f, const double delta) const {
    //     lhs >= rhs
    //
    // After strengthening, we have:
    //     (lhs >= rhs + delta)
    const Expression lhs{Visit(get_lhs_expression(f), delta)};
    const Expression rhs{Visit(get_rhs_expression(f), delta)};
    if (is_variable(rhs)) {
      // We return the following so that possibly we can keep the
      // bounded constraint form (c relop. v) where c is a constant.
      // Keeping this syntactic form is useful because our
      // FilterAssertion relies on that.
      return (lhs - delta >= rhs);
    } else {
      return (lhs >= rhs + delta);
    }
  }
  [[nodiscard]] Formula VisitLessThan(const Formula &f, const double delta) const {
    //     lhs < rhs
    //
    // After strengthening, we have:
    //     (lhs + delta < rhs)
    const Expression lhs{Visit(get_lhs_expression(f), delta)};
    const Expression rhs{Visit(get_rhs_expression(f), delta)};
    if (is_variable(lhs)) {
      // We return the following so that possibly we can keep the
      // bounded constraint form (v relop. c) where c is a constant.
      // Keeping this syntactic form is useful because our
      // FilterAssertion relies on that.
      return (lhs < rhs - delta);
    } else {
      return (lhs + delta < rhs);
    }
  }
  [[nodiscard]] Formula VisitLessThanOrEqualTo(const Formula &f, const double delta) const {
    //     lhs <= rhs
    //
    // After strengthening, we have:
    //     (lhs + delta <= rhs)
    const Expression lhs{Visit(get_lhs_expression(f), delta)};
    const Expression rhs{Visit(get_rhs_expression(f), delta)};
    if (is_variable(lhs)) {
      // We return the following so that possibly we can keep the
      // bounded constraint form (v relop. c) where c is a constant.
      // Keeping this syntactic form is useful because our
      // FilterAssertion relies on that.
      return (lhs <= rhs - delta);
    } else {
      return (lhs + delta <= rhs);
    }
  }
  [[nodiscard]] Formula VisitConjunction(const Formula &f, const double delta) const {
    Formula ret{Formula::True()};
    for (const auto &f_i : get_operands(f)) {
      ret = std::move(ret) && this->Visit(f_i, delta);
    }
    return ret;
  }
  [[nodiscard]] Formula VisitDisjunction(const Formula &f, const double delta) const {
    Formula ret{Formula::False()};
    for (const auto &f_i : get_operands(f)) {
      ret = std::move(ret) || this->Visit(f_i, delta);
    }
    return ret;
  }
  [[nodiscard]] Formula VisitNegation(const Formula &f, const double delta) const {
    return !Visit(get_operand(f), -delta);
  }
  [[nodiscard]] Formula VisitForall(const Formula &, const double) const {
    SMATS_RUNTIME_ERROR("DeltaStrengthenVisitor: forall formula is not supported.");
  }

  // Makes VisitExpression a friend of this class so that it can use private
  // operator()s.
  friend Expression drake::symbolic::VisitExpression<Expression>(const DeltaStrengthenVisitor *, const Expression &,
                                                                 const double &);

  // Makes VisitFormula a friend of this class so that it can use private
  // operator()s.
  friend Formula drake::symbolic::VisitFormula<Formula>(const DeltaStrengthenVisitor *, const Formula &,
                                                        const double &);
};

/// Visitor class which strengthens a formula by delta.
class IsDifferentiableVisitor {
 public:
  IsDifferentiableVisitor() = default;
  [[nodiscard]] bool Visit(const Formula &f) const { return VisitFormula<bool>(this, f); }
  [[nodiscard]] bool Visit(const Expression &e) const { return VisitExpression<bool>(this, e); }

 private:
  // Handle Formulas.
  [[nodiscard]] bool VisitFalse(const Formula &) const { return true; }
  [[nodiscard]] bool VisitTrue(const Formula &) const { return true; }
  [[nodiscard]] bool VisitVariable(const Formula &) const { return true; }
  [[nodiscard]] bool VisitEqualTo(const Formula &f) const {
    return Visit(get_lhs_expression(f)) && Visit(get_rhs_expression(f));
  }
  [[nodiscard]] bool VisitNotEqualTo(const Formula &f) const {
    return Visit(get_lhs_expression(f)) && Visit(get_rhs_expression(f));
  }
  [[nodiscard]] bool VisitGreaterThan(const Formula &f) const {
    return Visit(get_lhs_expression(f)) && Visit(get_rhs_expression(f));
  }
  [[nodiscard]] bool VisitGreaterThanOrEqualTo(const Formula &f) const {
    return Visit(get_lhs_expression(f)) && Visit(get_rhs_expression(f));
  }
  [[nodiscard]] bool VisitLessThan(const Formula &f) const {
    return Visit(get_lhs_expression(f)) && Visit(get_rhs_expression(f));
  }
  [[nodiscard]] bool VisitLessThanOrEqualTo(const Formula &f) const {
    return Visit(get_lhs_expression(f)) && Visit(get_rhs_expression(f));
  }
  [[nodiscard]] bool VisitConjunction(const Formula &f) const {
    std::set<Formula> formulae = get_operands(f);
    return std::all_of(formulae.begin(), formulae.end(), [this](const Formula &formula) { return Visit(formula); });
  }
  [[nodiscard]] bool VisitDisjunction(const Formula &f) const {
    std::set<Formula> formulae = get_operands(f);
    return std::all_of(formulae.begin(), formulae.end(), [this](const Formula &formula) { return Visit(formula); });
  }
  [[nodiscard]] bool VisitNegation(const Formula &f) const { return Visit(get_operand(f)); }
  [[nodiscard]] bool VisitForall(const Formula &) const { return false; }

  // Handle Expressions.
  [[nodiscard]] bool VisitVariable(const Expression &) const { return true; }
  [[nodiscard]] bool VisitConstant(const Expression &) const { return true; }
  [[nodiscard]] bool VisitAddition(const Expression &e) const {
    for (const auto &p : get_expr_to_coeff_map_in_addition(e)) {
      const Expression &e_i{p.first};
      if (!Visit(e_i)) {
        return false;
      }
    }
    return true;
  }
  [[nodiscard]] bool VisitMultiplication(const Expression &e) const {
    for (const auto &p : get_base_to_exponent_map_in_multiplication(e)) {
      const Expression &base{p.first};
      const Expression &exponent{p.second};
      if (!Visit(base) || !Visit(exponent)) {
        return false;
      }
    }
    return true;
  }
  [[nodiscard]] bool VisitDivision(const Expression &e) const {
    return Visit(get_first_argument(e)) && Visit(get_second_argument(e));
  }
  [[nodiscard]] bool VisitLog(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitAbs(const Expression &) const { return false; }
  [[nodiscard]] bool VisitExp(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitSqrt(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitPow(const Expression &e) const {
    return Visit(get_first_argument(e)) && Visit(get_second_argument(e));
  }
  [[nodiscard]] bool VisitSin(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitCos(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitTan(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitAsin(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitAcos(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitAtan(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitAtan2(const Expression &e) const {
    return Visit(get_first_argument(e)) && Visit(get_second_argument(e));
  }
  [[nodiscard]] bool VisitSinh(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitCosh(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitTanh(const Expression &e) const { return Visit(get_argument(e)); }
  [[nodiscard]] bool VisitMin(const Expression &) const { return false; }
  [[nodiscard]] bool VisitMax(const Expression &) const { return false; }
  [[nodiscard]] bool VisitIfThenElse(const Expression &) const { return false; }
  [[nodiscard]] bool VisitUninterpretedFunction(const Expression &) const { return false; }

  // Makes VisitFormula a friend of this class so that it can use private
  // operator()s.
  friend bool drake::symbolic::VisitFormula<bool>(const IsDifferentiableVisitor *, const Formula &);
  // Makes VisitExpression a friend of this class so that it can use private
  // operator()s.
  friend bool drake::symbolic::VisitExpression<bool>(const IsDifferentiableVisitor *, const Expression &);
};

}  // namespace

/// Strengthen the input formula $p f by @p delta.
Formula DeltaStrengthen(const Formula &f, const double delta) {
  SMATS_ASSERT(delta > 0, "delta must be positive.");
  return DeltaStrengthenVisitor{}.Strengthen(f, delta);
}

/// Weaken the input formula $p f by @p delta.
Formula DeltaWeaken(const Formula &f, const double delta) {
  SMATS_ASSERT(delta > 0, "delta must be positive.");
  return DeltaStrengthenVisitor{}.Strengthen(f, -delta);
}

bool IsDifferentiable(const Formula &f) { return IsDifferentiableVisitor{}.Visit(f); }

bool IsDifferentiable(const Expression &e) { return IsDifferentiableVisitor{}.Visit(e); }

Formula make_conjunction(const std::vector<Formula> &formulas) {
  Formula ret{Formula::True()};
  for (const auto &f_i : formulas) {
    ret = std::move(ret) && f_i;
  }
  return ret;
}

Formula make_disjunction(const std::vector<Formula> &formulas) {
  Formula ret{Formula::False()};
  for (const auto &f_i : formulas) {
    ret = std::move(ret) || f_i;
  }
  return ret;
}

std::vector<Variable> CreateVector(const std::string &prefix, const int size, const Variable::Type type) {
  SMATS_ASSERT(!prefix.empty(), "prefix must not be empty.");
  SMATS_ASSERT(size >= 1, "size must be positive.");
  std::vector<Variable> v;
  v.reserve(size);
  for (int i = 0; i < size; ++i) {
    v.emplace_back(prefix + std::to_string(i), type);
  }
  return v;
}

RelationalOperator operator!(const RelationalOperator op) {
  switch (op) {
    case RelationalOperator::EQ:
      return RelationalOperator::NEQ;
    case RelationalOperator::NEQ:
      return RelationalOperator::EQ;
    case RelationalOperator::GT:
      return RelationalOperator::LEQ;
    case RelationalOperator::GEQ:
      return RelationalOperator::LT;
    case RelationalOperator::LT:
      return RelationalOperator::GEQ;
    case RelationalOperator::LEQ:
      return RelationalOperator::GT;
    default:
      SMATS_UNREACHABLE();
  }
}

std::ostream &operator<<(std::ostream &os, const RelationalOperator op) {
  switch (op) {
    case RelationalOperator::EQ:
      return os << "=";
    case RelationalOperator::NEQ:
      return os << "≠";
    case RelationalOperator::GT:
      return os << ">";
    case RelationalOperator::GEQ:
      return os << "≥";
    case RelationalOperator::LT:
      return os << "<";
    case RelationalOperator::LEQ:
      return os << "≤";
    default:
      SMATS_UNREACHABLE();
  }
}

std::ostream &operator<<(std::ostream &os, const FormulaKind &kind) {
  switch (kind) {
    case FormulaKind::False:
      return os << "False";
    case FormulaKind::True:
      return os << "True";
    case FormulaKind::Var:
      return os << "Var";
    case FormulaKind::Eq:
      return os << "Eq";
    case FormulaKind::Neq:
      return os << "Neq";
    case FormulaKind::Gt:
      return os << "Gt";
    case FormulaKind::Geq:
      return os << "Geq";
    case FormulaKind::Lt:
      return os << "Lt";
    case FormulaKind::Leq:
      return os << "Leq";
    case FormulaKind::And:
      return os << "And";
    case FormulaKind::Or:
      return os << "Or";
    case FormulaKind::Not:
      return os << "Not";
    case FormulaKind::Forall:
      return os << "Forall";
    default:
      SMATS_UNREACHABLE();
  }
}

std::ostream &operator<<(std::ostream &os, const ExpressionKind &kind) {
  switch (kind) {
    case ExpressionKind::Var:
      return os << "Var";
    case ExpressionKind::Constant:
      return os << "Constant";
    case ExpressionKind::Infty:
      return os << "Infty";
    case ExpressionKind::NaN:
      return os << "NaN";
    case ExpressionKind::Add:
      return os << "Addition";
    case ExpressionKind::Mul:
      return os << "Multiplication";
    case ExpressionKind::Div:
      return os << "Division";
    case ExpressionKind::Log:
      return os << "Log";
    case ExpressionKind::Abs:
      return os << "Abs";
    case ExpressionKind::Exp:
      return os << "Exp";
    case ExpressionKind::Sqrt:
      return os << "Sqrt";
    case ExpressionKind::Pow:
      return os << "Pow";
    case ExpressionKind::Sin:
      return os << "Sin";
    case ExpressionKind::Cos:
      return os << "Cos";
    case ExpressionKind::Tan:
      return os << "Tan";
    case ExpressionKind::Asin:
      return os << "Asin";
    case ExpressionKind::Acos:
      return os << "Acos";
    case ExpressionKind::Atan:
      return os << "Atan";
    case ExpressionKind::Atan2:
      return os << "Atan2";
    case ExpressionKind::Sinh:
      return os << "Sinh";
    case ExpressionKind::Cosh:
      return os << "Cosh";
    case ExpressionKind::Tanh:
      return os << "Tanh";
    case ExpressionKind::Min:
      return os << "Min";
    case ExpressionKind::Max:
      return os << "Max";
    case ExpressionKind::IfThenElse:
      return os << "IfThenElse";
    case ExpressionKind::UninterpretedFunction:
      return os << "UninterpretedFunction";
    default:
      SMATS_UNREACHABLE();
  }
}

}  // namespace smats
