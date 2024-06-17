/**
 * @author smats
 * @date 15 Aug 2023
 * @copyright 2023 smats
 * @brief NNF (Negation Normal Form) conversion.
 *
 * NNFizer class converts a formula into an equivalent formula in NNF.
 * @see https://en.wikipedia.org/wiki/Negation_normal_form
 */
#pragma once

#include "smats/symbolic/symbolic.h"

namespace smats {

/**
 * NNFizer class
 *
 * Implementation of NNF (Negation Normal Form) conversion.
 * When @code push_negation_into_relationals @endcode is true, it pushed
 * negations into relational formulas by flipping relational
 * @par Example:
 *   @f$ ¬(x \ge 10) @f$ becomes @f$ (x < 10) @f$
 *
 * @see https://en.wikipedia.org/wiki/Negation_normal_form
 */
class Nnfizer {
 public:
  /**
   * Convert a @p f into an equivalent formula @c f' in NNF.
   * @param f Formula to be converted.
   * @param push_negation_into_relationals Push negation into relational formulas.
   * @return Converted formula.
   */
  [[nodiscard]] Formula Convert(const Formula &f, bool push_negation_into_relationals = false) const;

 private:
  /**
   * Convert @p f into an equivalent formula @c f' in NNF. The parameter @p polarity
   * is to indicate whether it processes @c f (if @p polarity is true) or @c ¬f (if @p polarity is false).
   * @param f Formula to be converted.
   * @param polarity Polarity.
   * @param push_negation_into_relationals Push negation into relational formulas.
   * @return Converted formula.
   */
  [[nodiscard]] Formula Visit(const Formula &f, bool polarity, bool push_negation_into_relationals) const;

  [[nodiscard]] Formula VisitFalse(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitTrue(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitVariable(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitEqualTo(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitNotEqualTo(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitGreaterThan(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitGreaterThanOrEqualTo(const Formula &f, bool polarity,
                                                  bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitLessThan(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitLessThanOrEqualTo(const Formula &f, bool polarity,
                                               bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitConjunction(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitDisjunction(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitNegation(const Formula &f, bool polarity, bool push_negation_into_relationals) const;
  [[nodiscard]] Formula VisitForall(const Formula &f, bool polarity, bool push_negation_into_relationals) const;

  // Makes VisitFormula a friend of this class so that it can use private
  // methods.
  friend Formula drake::symbolic::VisitFormula<Formula>(const Nnfizer *, const Formula &, const bool &, const bool &);
};
}  // namespace smats
