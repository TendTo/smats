/**
 * @author smats
 * @date 18 Aug 2023
 * @copyright 2023 smats
 * @brief Brief description
 *
 * Long Description
 */
#pragma once

#include "smats/symbolic/Nnfizer.h"
#include "smats/symbolic/symbolic.h"

namespace smats {

/// Transforms a symbolic formula @p f into a CNF formula by
/// preserving its semantics.
///
/// @note This transform can increase the size exponentially. We are
/// using this transformation in TseitinCnfizer when we process the
/// nested formula in a universally quantified formula.
class NaiveCnfizer {
 public:
  /// Convert @p f into its CNF form.
  [[nodiscard]] Formula Convert(const Formula &f) const;

 private:
  [[nodiscard]] Formula Visit(const Formula &f) const;
  [[nodiscard]] Formula VisitFalse(const Formula &f) const;
  [[nodiscard]] Formula VisitTrue(const Formula &f) const;
  [[nodiscard]] Formula VisitVariable(const Formula &f) const;
  [[nodiscard]] Formula VisitEqualTo(const Formula &f) const;
  [[nodiscard]] Formula VisitNotEqualTo(const Formula &f) const;
  [[nodiscard]] Formula VisitGreaterThan(const Formula &f) const;
  [[nodiscard]] Formula VisitGreaterThanOrEqualTo(const Formula &f) const;
  [[nodiscard]] Formula VisitLessThan(const Formula &f) const;
  [[nodiscard]] Formula VisitLessThanOrEqualTo(const Formula &f) const;
  [[nodiscard]] Formula VisitConjunction(const Formula &f) const;
  [[nodiscard]] Formula VisitDisjunction(const Formula &f) const;
  [[nodiscard]] Formula VisitNegation(const Formula &f) const;
  [[nodiscard]] Formula VisitForall(const Formula &f) const;

  const Nnfizer nnfizer_{};

  // Makes VisitFormula a friend of this class so that it can use private
  // operator()s.
  friend Formula drake::symbolic::VisitFormula<Formula>(const NaiveCnfizer *, const Formula &);
};

}  // namespace smats
