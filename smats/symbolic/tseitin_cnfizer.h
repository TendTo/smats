/**
 * @author smats
 * @date 19 Aug 2023
 * @copyright 2023 smats
 * @brief Brief description
 *
 * Long Description
 */
#pragma once

#include <map>
#include <vector>

#include "smats/symbolic/FormulaVisitor.h"
#include "smats/symbolic/NaiveCnfizer.h"
#include "smats/symbolic/literal.h"
#include "smats/symbolic/symbolic.h"
#include "smats/util/Config.h"

namespace smats {

/**
 * Tsietin transformation is a method to convert a formula into an
 * equi-satisfiable formula in CNF. The method introduces extra Boolean
 * variables (Tseitin transformation).
 */
class TseitinCnfizer : public FormulaVisitor {
 public:
  explicit TseitinCnfizer(const Config &config) : FormulaVisitor{config, "TseitinCnfizer"} {}

  /**
   * Convert @p f into an equi-satisfiable formula @c f' in CNF.
   * @param f formula to convert
   * @return equi-satisfiable formula in CNF
   */
  std::vector<Formula> Convert(const Formula &f);

  /**
   * Return a const reference of `map_` member.
   * @note map_ is cleared at the beginning of `Convert` call.
   * @return const reference of `map_` member.n
   */
  [[nodiscard]] const std::map<Variable, Formula> &map() const { return map_; }

 private:
  Formula Visit(const Formula &f) override;
  Formula VisitConjunction(const Formula &f) override;
  Formula VisitDisjunction(const Formula &f) override;
  Formula VisitNegation(const Formula &f) override;
  Formula VisitForall(const Formula &f) override;

  /**
   * Map a temporary variable, which is introduced by a Tseitin
   * transformation, to a corresponding Formula.
   *
   * @note that this map_ is cleared at the beginning of `Convert`
   * call.
   */
  std::map<Variable, Formula> map_;

  const NaiveCnfizer naive_cnfizer_{};  ///< Naive CNFizer. Transforms nested formulas inside universal quantification.

  // Makes VisitFormula a friend of this class so that it can use private
  // operator()s.
  friend Formula drake::symbolic::VisitFormula<Formula, TseitinCnfizer>(TseitinCnfizer *, const Formula &);
};

}  // namespace smats
