/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * @brief LinearFormulaFlattener class
 *
 * Used by the @ref smats::PredicateAbstractor to ensure that all the theory literals are
 * in the flattened standard form.
 * @see LinearFormulaFlattener::Flatten
 */
#pragma once

#include "smats/symbolic/symbolic.h"
#include "smats/util/Config.h"

namespace smats {

/**
 * Used by the @ref PredicateAbstractor to ensure that all the theory literals are
 * in the flattened standard form.
 * @see LinearFormulaFlattener::Flatten
 */
class LinearFormulaFlattener {
 public:
  /**
   *
   * @param config
   */
  explicit LinearFormulaFlattener(const Config& config) : config_{config} {}
  /**
   * Flatten the given formula.
   *
   * A formula is considered flatten if:
   * - is in the form @f$ a_1x_1 + a_2x_2 + \dots a_nx_n \lessgtr c @f$ where @f$ a_i, c_i \in \mathbb{R} @f$ are
   * constants, @f$ a_1 > 0 @f$ and @f$ x_i \in \mathbb{R} @f$ is a variable @f$ \forall i \in \{1, 2, \dots, n\} @f$
   * - calling the method Expand on both of the formula's terms outputs the same expression as the
   * one used as the input
   * @warning The formula returned has a very limited lifetime, being a reference of either the input @p formula or
   * @ref flattered_formula_ , whose value may be overwritten at the next invocation of this method.
   * @param formula the formula to flatten.
   * @return a reference to the flattened formula.
   */
  [[nodiscard]] Formula Flatten(const Formula& formula) const;

 private:
  /**
   * Use the updated expressions to build a new formula, also removing any mult expression from the left-hand-side.
   *
   * If a formula is in the form @f$ a x \lessgtr b @f$ where @f$ a, b \in \mathbb{R}, a \ne 0 @f$ are constants
   * and @f$ x \in \mathbb{R} @f$ is an unknown variable, this method will remove the multiplication
   * from the left-hand-side of the formula by dividing both sides by @f$ a @f$.
   * If a formula is in the form @f$ a_1x_1 + a_2x_2 + \dots a_nx_n \lessgtr c @f$ where @f$ a_i, c_i \in \mathbb{R} @f$
   * are constants and @f$ x_i \in \mathbb{R} @f$ is an unknown variable @f$ \forall i \in \{1, 2, \dots, n\} @f$,
   * this method will ensure that the first term in the addition of the left-hand-side has a positive coefficient, by
   * multiplying both sides by @f$ -1 @f$ if necessary.
   * @param lhs left-hand-side expression of the new formula
   * @param rhs right-hand-side expression of the new formula
   * @param kind kind of the formula (e.g. Eq, Lt, Geq, ...)
   */
  [[nodiscard]] Formula BuildFlatteredFormula(const Expression& lhs, const Expression& rhs, FormulaKind kind) const;

  const Config& config_;       ///< Configuration
};

}  // namespace smats
