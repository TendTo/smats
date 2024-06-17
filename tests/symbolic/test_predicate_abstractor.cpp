/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include "smats/symbolic/predicate_abstractor.h"

using std::set;

namespace smats {

class TestPredicateAbstractor : public ::testing::Test {
  DrakeSymbolicGuard guard_;
  Config config_;

 protected:
  const Variable x_{"x", Variable::Type::CONTINUOUS};
  const Variable y_{"y", Variable::Type::CONTINUOUS};
  const Variable z_{"z", Variable::Type::CONTINUOUS};

  const Variable b1_{"b1", Variable::Type::BOOLEAN};
  const Variable b2_{"b2", Variable::Type::BOOLEAN};
  const Variable b3_{"b3", Variable::Type::BOOLEAN};

  PredicateAbstractor abstractor_{config_};
};

TEST_F(TestPredicateAbstractor, False) {
  // False <-> False
  const Formula abstracted{abstractor_.Convert(Formula::False())};
  EXPECT_TRUE(is_false(abstracted));
}

TEST_F(TestPredicateAbstractor, True) {
  // True <-> True
  const Formula abstracted{abstractor_.Convert(Formula::True())};
  EXPECT_TRUE(is_true(abstracted));
}

TEST_F(TestPredicateAbstractor, Variable) {
  // b <-> b
  const Formula f{b1_};
  const Formula f_abstracted{abstractor_.Convert(f)};
  EXPECT_PRED2(FormulaEqual, f, f_abstracted);
}

TEST_F(TestPredicateAbstractor, Eq) {
  // x + y == 10 <-> b
  const Formula f{x_ + y_ == 10};
  const Formula f_abstracted{abstractor_.Convert(f)};

  EXPECT_FALSE(is_variable(f));
  ASSERT_TRUE(is_variable(f_abstracted));
  const Variable &var{get_variable(f_abstracted)};

  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], f);
  EXPECT_PRED2(VarEqual, abstractor_[f], var);
}

TEST_F(TestPredicateAbstractor, Neq) {
  // x + y != 10 <-> !(x + y == 10)
  const Formula f{x_ + y_ != 10};
  const Formula f_abstracted{abstractor_.Convert(f)};

  EXPECT_FALSE(is_variable(f));
  ASSERT_TRUE(is_negation(f_abstracted));
  ASSERT_TRUE(is_variable(get_operand(f_abstracted)));
  const Variable &var{get_variable(get_operand(f_abstracted))};

  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], x_ + y_ == 10);
  EXPECT_PRED2(VarEqual, abstractor_[x_ + y_ == 10], var);
}

TEST_F(TestPredicateAbstractor, Gt) {
  // x + y > 10 <-> !(x + y <= 10)
  const Formula f{x_ + y_ > 10};
  const Formula f_abstracted{abstractor_.Convert(f)};

  EXPECT_FALSE(is_variable(f));
  ASSERT_TRUE(is_negation(f_abstracted));
  ASSERT_TRUE(is_variable(get_operand(f_abstracted)));
  const Variable &var{get_variable(get_operand(f_abstracted))};

  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], x_ + y_ <= 10);
  EXPECT_PRED2(VarEqual, abstractor_[x_ + y_ <= 10], var);
}

TEST_F(TestPredicateAbstractor, Geq) {
  // x + y >= 10 <-> !(x + y < 10)
  const Formula f{x_ + y_ >= 10};
  const Formula f_abstracted{abstractor_.Convert(f)};

  EXPECT_FALSE(is_variable(f));
  ASSERT_TRUE(is_negation(f_abstracted));
  ASSERT_TRUE(is_variable(get_operand(f_abstracted)));
  const Variable &var{get_variable(get_operand(f_abstracted))};

  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], x_ + y_ < 10);
  EXPECT_PRED2(VarEqual, abstractor_[x_ + y_ < 10], var);
}

TEST_F(TestPredicateAbstractor, Lt) {
  // x + y < 10 <-> b
  const Formula f{x_ + y_ < 10};
  const Formula f_abstracted{abstractor_.Convert(f)};

  EXPECT_FALSE(is_variable(f));
  ASSERT_TRUE(is_variable(f_abstracted));
  const Variable &var{get_variable(f_abstracted)};

  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], f);
  EXPECT_PRED2(VarEqual, abstractor_[f], var);
}

TEST_F(TestPredicateAbstractor, Leq) {
  // x + y <= 10 <-> b
  const Formula f{x_ + y_ <= 10};
  const Formula f_abstracted{abstractor_.Convert(f)};

  EXPECT_FALSE(is_variable(f));
  ASSERT_TRUE(is_variable(f_abstracted));
  const Variable &var{get_variable(f_abstracted)};

  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], f);
  EXPECT_PRED2(VarEqual, abstractor_[f], var);
}

TEST_F(TestPredicateAbstractor, Conjunction) {
  // f₁ ∧ f₂ ∧ f₃ ∧ f₄ <-> !b₁ ∧ !b₂ ∧ b₃ ∧ !b₄
  const Formula f1{x_ >= y_};
  const Formula f2{y_ >= z_};
  const Formula f3{x_ + y_ <= 15};
  const Formula f4{y_ * z_ > 0};
  const Formula conjunction{f1 && f2 && f3 && f4};
  const set<Formula> operands{get_operands(conjunction)};

  const Formula conjunction_abstracted{abstractor_.Convert(conjunction)};
  ASSERT_TRUE(is_conjunction(conjunction_abstracted));
  const set<Formula> operands_abstracted{get_operands(conjunction_abstracted)};
  EXPECT_EQ(operands_abstracted.size(), operands.size());

  for (const Formula &f : operands_abstracted) {
    ASSERT_TRUE(is_variable(f) || (is_negation(f) && is_variable(get_operand(f))));
    // if (is_variable(f)) {
    //   // Case: f = v.
    //   const Variable& var{get_variable(f)};
    //   const Formula& corresponding_f{abstractor_[var]};
    //   EXPECT_TRUE(operands.find(corresponding_f) != operands.end());
    // } else {
    //   // Case: f = !v.
    //   const Variable& var{get_variable(get_operand(f))};
    //   const Formula& corresponding_f{abstractor_[var]};
    //   EXPECT_TRUE(operands.find(corresponding_f) != operands.end());
    // }
  }
}

TEST_F(TestPredicateAbstractor, Disjunction) {
  // f₁ ∨ f₂ ∨ f₃ ∨ f₄ <-> b₁ ∨ b₂ ∨ b₃ ∨ b₄
  const Formula f1{x_ >= y_};
  const Formula f2{y_ >= z_};
  const Formula f3{x_ + y_ <= 15};
  const Formula f4{y_ * z_ > 0};
  const Formula disjunction{f1 || f2 || f3 || f4};
  const set<Formula> operands{get_operands(disjunction)};

  const Formula disjunction_abstracted{abstractor_.Convert(disjunction)};
  ASSERT_TRUE(is_disjunction(disjunction_abstracted));
  const set<Formula> operands_abstracted{get_operands(disjunction_abstracted)};
  EXPECT_EQ(operands_abstracted.size(), operands.size());

  for (const Formula &f : operands_abstracted) {
    ASSERT_TRUE(is_variable(f) || (is_negation(f) && is_variable(get_operand(f))));
    // ASSERT_TRUE(is_variable(f));
    // const Variable& var{get_variable(f)};
    // const Formula& corresponding_f{abstractor_[var]};
    // EXPECT_TRUE(operands.find(corresponding_f) != operands.end());
  }
}

TEST_F(TestPredicateAbstractor, Negation) {
  // !(x + y >= 10) <-> x + y < 10
  const Formula f{x_ + y_ >= 10};
  const Formula not_f{!f};
  const Formula not_f_abstracted{abstractor_.Convert(not_f)};

  ASSERT_TRUE(is_variable(not_f_abstracted));
  const Variable &var{get_variable(not_f_abstracted)};
  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], x_ + y_ < 10);
  EXPECT_PRED2(VarEqual, abstractor_[x_ + y_ < 10], var);
}

TEST_F(TestPredicateAbstractor, Forall) {
  // (∀x. x >= 0) <-> b
  const Formula f{forall({x_}, x_ >= 0)};
  const Formula f_abstracted{abstractor_.Convert(f)};
  ASSERT_TRUE(is_variable(f_abstracted));
  const Variable &var{get_variable(f_abstracted)};
  EXPECT_EQ(var.get_type(), Variable::Type::BOOLEAN);
  EXPECT_PRED2(FormulaEqual, abstractor_[var], f);
  EXPECT_PRED2(VarEqual, abstractor_[f], var);
}
}  // namespace smats