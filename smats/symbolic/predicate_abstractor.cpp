/**
 * @author smats
 * @date 17 Aug 2023
 * @copyright 2023 smats
 */
#include "smats/symbolic/predicate_abstractor.h"

#include <set>
#include <sstream>
#include <utility>

#include "smats/symbolic/linearFormula_flattener.h"
#include "smats/util/stats.h"
#include "smats/util/timer.h"

namespace smats {

Formula PredicateAbstractor::Convert(const Formula &f) {
  TimerGuard timer_guard(&stats_.m_timer(), stats_.enabled());
  stats_.Increase();
  return Visit(f);
}

Formula PredicateAbstractor::Convert(const std::vector<Formula> &formulas) {
  return Convert(make_conjunction(std::set<Formula>{formulas.begin(), formulas.end()}));
}

Formula PredicateAbstractor::Visit(const Formula &f) {
  const Formula flattened_f{flattener_.Flatten(f)};
  const bool is_negated = is_negation(flattened_f);
  const Formula &unary_f = is_negated ? get_operand(flattened_f) : flattened_f;

  // First check if we processed this formula before.
  const auto it = formula_to_var_map_.find(unary_f);
  if (it == formula_to_var_map_.cend()) {
    // No, we haven't processed it before.
    return VisitFormula<Formula>(this, flattened_f);
  } else {
    // Yes, we have processed this formula before.
    return is_negated ? !Formula{it->second} : Formula{it->second};
  }
}

Formula PredicateAbstractor::VisitAtomic(const Formula &f) {
  // Flatten linear formulas to make sure they have the standard form (ax + by <=> c).
  const Formula flattened_f{flattener_.Flatten(f)};
  const bool is_negated = is_negation(flattened_f);
  const Formula &unary_f = is_negated ? get_operand(flattened_f) : flattened_f;

  auto it = formula_to_var_map_.find(unary_f);
  // Leaf case: create a new Boolean variable `bvar` and record the relation between `bvar` and `f`.
  if (it == formula_to_var_map_.end()) {
    const Variable bvar{(std::stringstream{} << "b(" << unary_f << ")").str(), Variable::Type::BOOLEAN};
    var_to_formula_map_.emplace(bvar, unary_f);
    formula_to_var_map_.emplace(unary_f, bvar);
    return is_negated ? !Formula{bvar} : Formula{bvar};
  }
  return is_negated ? !Formula{it->second} : Formula{it->second};
}

Formula PredicateAbstractor::VisitEqualTo(const Formula &f) { return VisitAtomic(f); }
Formula PredicateAbstractor::VisitNotEqualTo(const Formula &f) { return VisitAtomic(f); }
Formula PredicateAbstractor::VisitLessThan(const Formula &f) { return VisitAtomic(f); }
Formula PredicateAbstractor::VisitLessThanOrEqualTo(const Formula &f) { return VisitAtomic(f); }
Formula PredicateAbstractor::VisitGreaterThan(const Formula &f) { return VisitAtomic(f); }
Formula PredicateAbstractor::VisitGreaterThanOrEqualTo(const Formula &f) { return VisitAtomic(f); }

Formula PredicateAbstractor::VisitNegation(const Formula &f) { return !Visit(get_operand(f)); }
Formula PredicateAbstractor::VisitForall(const Formula &f) { return VisitAtomic(f); }
Formula PredicateAbstractor::VisitConjunction(const Formula &f) {
  const std::set<Formula> operands{
      map(get_operands(f), [this](const Formula &formula) { return this->Visit(formula); })};
  return make_conjunction(operands);
}
Formula PredicateAbstractor::VisitDisjunction(const Formula &f) {
  const std::set<Formula> operands{
      map(get_operands(f), [this](const Formula &formula) { return this->Visit(formula); })};
  return make_disjunction(operands);
}

}  // namespace smats
