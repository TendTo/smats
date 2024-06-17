/**
 * @author smats
 * @date 17 Aug 2023
 * @copyright 2023 smats
 * @brief Brief description
 *
 * Long Description
 */
#pragma once

#include <unordered_map>
#include <vector>

#include "smats/symbolic/formula_visitor.h"
#include "smats/symbolic/linear_formula_flattener.h"
#include "smats/symbolic/literal.h"
#include "smats/symbolic/symbolic.h"
#include "smats/util/config.h"

namespace smats {

class PredicateAbstractor : public FormulaVisitor {
 public:
  explicit PredicateAbstractor(const Config &config)
      : FormulaVisitor{config, "PredicateAbstractor"}, flattener_{config} {}
  /**
   * Convert a first-order logic formula @p f into a Boolean formula
   * by predicate abstraction. For example, a formula `(x > 0) ∧ (y <
   * 0)` will be converted into `b₁ ∧ b₂` while `b₁` corresponds with
   * `x > 0` and `b₂` corresponds with `y < 0`. The class provides
   * `operator[b]` which looks up the corresponding formula for a
   * Boolean variable `b`.
   * @param f formula to be converted.
   * @return boolean formula
   */
  Formula Convert(const Formula &f);

  /**
   * Convert a first-order logic formula @p f into a Boolean formula
   * by predicate abstraction. For example, a formula `(x > 0) ∧ (y <
   * 0)` will be converted into `b₁ ∧ b₂` while `b₁` corresponds with
   * `x > 0` and `b₂` corresponds with `y < 0`. The class provides
   * `operator[b]` which looks up the corresponding formula for a
   * Boolean variable `b`.
   * @param formulas formulas to be converted.
   * @return boolean formula
   */
  Formula Convert(const std::vector<Formula> &formulas);

  const std::unordered_map<Variable, Formula, hash_value<Variable>> &var_to_formula_map() const {
    return var_to_formula_map_;
  }

  const Variable &operator[](const Formula &f) const { return formula_to_var_map_.at(f); }

  const Formula &operator[](const Variable &var) const { return var_to_formula_map_.at(var); }

 private:
  Formula Visit(const Formula &f) override;
  /**
   * Visit an atomic formula.
   *
   * It flattens the formula and creates a new Boolean variable if the formula is not present in the map.
   * Otherwise, it returns the corresponding Boolean variable.
   * @param f atomic formula to visit
   * @return newly created Boolean variable in the map @ref var_to_formula_map_ if the formula is not present
   * @return existing Boolean variable in the map @ref var_to_formula_map_ if the formula was already present
   */
  Formula VisitAtomic(const Formula &f);
  Formula VisitEqualTo(const Formula &f) override;
  Formula VisitNotEqualTo(const Formula &f) override;
  Formula VisitGreaterThan(const Formula &f) override;
  Formula VisitGreaterThanOrEqualTo(const Formula &f) override;
  Formula VisitLessThan(const Formula &f) override;
  Formula VisitLessThanOrEqualTo(const Formula &f) override;
  Formula VisitConjunction(const Formula &f) override;
  Formula VisitDisjunction(const Formula &f) override;
  Formula VisitNegation(const Formula &f) override;
  Formula VisitForall(const Formula &f) override;

  std::unordered_map<Variable, Formula, hash_value<Variable>> var_to_formula_map_;
  std::unordered_map<Formula, Variable> formula_to_var_map_;
  const LinearFormulaFlattener flattener_;

  // Makes VisitFormula a friend of this class so that it can use private
  // operator()s.
  friend Formula drake::symbolic::VisitFormula<Formula, PredicateAbstractor>(PredicateAbstractor *, const Formula &);
};

}  // namespace smats
