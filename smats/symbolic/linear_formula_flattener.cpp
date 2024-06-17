/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include "smats/symbolic/linear_formula_flattener.h"

#include <map>
#include <utility>

#include "smats/libs/libgmp.h"
#include "smats/util/exception.h"

namespace smats {

Formula LinearFormulaFlattener::Flatten(const Formula& formula) const {
  if (!is_relational(formula)) return formula;
  const bool needs_expansion_{config_.needs_expansion()};
  const Expression& lhs = get_lhs_expression(formula);
  const Expression& rhs = get_rhs_expression(formula);
  const Expression expr{needs_expansion_ ? (lhs - rhs).Expand() : lhs - rhs};
  SMATS_ASSERT_FMT(expr.EqualTo(expr.Expand()), "Expression {} must be expanded", formula);
  SMATS_ASSERT_FMT(is_addition(expr) || is_multiplication(expr) || is_variable(expr),
                     "Expression must be an addition, multiplication or a variable. Instead found {}", expr);

  if (is_addition(expr)) {
    const mpq_class& constant{get_constant_in_addition(expr)};
    return BuildFlatteredFormula(expr - constant, Expression{-constant}, formula.get_kind());
  }
  return BuildFlatteredFormula(expr, 0, formula.get_kind());
}

Formula LinearFormulaFlattener::BuildFlatteredFormula(const Expression& lhs, const Expression& rhs,
                                                      FormulaKind kind) const {
  // Remove multiplication from the left-hand-side of the formula if they are of the form a*x <=> b
  if (is_multiplication(lhs) && get_base_to_exponent_map_in_multiplication(lhs).size() == 1 &&
      is_variable(get_base_to_exponent_map_in_multiplication(lhs).begin()->first)) {
    const mpq_class& constant{get_constant_in_multiplication(lhs)};
    // If the constant is 1, we can just return the formula as is, removing the multiplication.
    if (constant == 1)
      return BuildFlatteredFormula(get_base_to_exponent_map_in_multiplication(lhs).begin()->first, rhs, kind);
    SMATS_ASSERT(constant != 0, "Multiplication constant must be non-zero");
    const Expression coefficient{1 / constant};
    return BuildFlatteredFormula(lhs * coefficient, rhs * coefficient, coefficient >= 0 ? kind : -kind);
  }

  // Ensure the first term in the addition of the left-hand-side has a positive coefficient
  if (is_addition(lhs) && !get_expr_to_coeff_map_in_addition(lhs).empty() &&
      is_variable(get_expr_to_coeff_map_in_addition(lhs).begin()->first) &&
      get_expr_to_coeff_map_in_addition(lhs).begin()->second < 0) {
    return BuildFlatteredFormula(lhs * -1, rhs * -1, -kind);
  }

  SMATS_ASSERT_FMT(!is_multiplication(lhs) || get_base_to_exponent_map_in_multiplication(lhs).size() != 1 ||
                         !is_variable(get_base_to_exponent_map_in_multiplication(lhs).begin()->first),
                     "lhs {} should have been modified by a previous call as a mult", lhs);
  SMATS_ASSERT_FMT(!is_addition(lhs) || get_expr_to_coeff_map_in_addition(lhs).empty() ||
                         !is_variable(get_expr_to_coeff_map_in_addition(lhs).begin()->first) ||
                         get_expr_to_coeff_map_in_addition(lhs).begin()->second >= 0,
                     "lhs {} should have been modified by a previous call as an addition", lhs);

  switch (kind) {
    case FormulaKind::Eq:
      return lhs == rhs;
    case FormulaKind::Neq:
      return !(lhs == rhs);
    case FormulaKind::Leq:
      return lhs <= rhs;
    case FormulaKind::Lt:
      return lhs < rhs;
    case FormulaKind::Geq:
      return !(lhs < rhs);
    case FormulaKind::Gt:
      return !(lhs <= rhs);
    default:
      SMATS_UNREACHABLE();
  }
}

}  // namespace smats
