/**
 * @file TestArgparser.cpp
 * @author myapp (https://github.com/TendTo/myapp)
 * @copyright 2024 myapp
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include "myapp/util/calculator.h"

using myapp::Calculator;

class TestCalculator : public ::testing::Test {
 protected:
  myapp::Calculator calculator{};
};

TEST_F(TestCalculator, DefaultConstructor) {
  Calculator c{};
  EXPECT_EQ(c.getVerbose(), 0);
}

TEST_F(TestCalculator, VerboseConstructor) {
  int verbose = 1;
  Calculator c{verbose};
  EXPECT_EQ(c.getVerbose(), verbose);
}

TEST_F(TestCalculator, Addition) { EXPECT_EQ(calculator.add(1, 2), 3); }

TEST_F(TestCalculator, Subtraction) { EXPECT_EQ(calculator.subtract(1, 2), -1); }

TEST_F(TestCalculator, Multiplication) { EXPECT_EQ(calculator.multiply(2, 3), 6); }

TEST_F(TestCalculator, Division) { EXPECT_EQ(calculator.divide(6, 3), 2); }

TEST_F(TestCalculator, DivisionByZero) { EXPECT_EQ(calculator.divide(6, 0), 0); }
