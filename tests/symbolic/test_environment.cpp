#include <gtest/gtest.h>

#include "smats/symbolic/environment.h"

using smats::Environment;
using smats::Variable;
using smats::Variables;

class TestEnvironment : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  Environment env_;
};

TEST_F(TestEnvironment, InsertAndAccess) {
  env_.insert(x_, 2.0);
  env_.insert(y_, 3.0);

  EXPECT_EQ(env_.at(x_), 2.0);
  EXPECT_EQ(env_.at(y_), 3.0);
}

TEST_F(TestEnvironment, InsertOrAssign) {
  env_.insert_or_assign(x_, 2.0);
  env_.insert_or_assign(y_, 3.0);
  env_.insert_or_assign(x_, 4.0);

  EXPECT_EQ(env_.at(x_), 4.0);
  EXPECT_EQ(env_.at(y_), 3.0);
}

TEST_F(TestEnvironment, FindExistingKey) {
  env_.insert(x_, 2.0);
  env_.insert(y_, 3.0);

  auto it_x = env_.find(x_);
  auto it_y = env_.find(y_);

  EXPECT_NE(it_x, env_.end());
  EXPECT_NE(it_y, env_.end());
  EXPECT_EQ(it_x->second, 2.0);
  EXPECT_EQ(it_y->second, 3.0);
}

TEST_F(TestEnvironment, FindNonExistingKey) {
  env_.insert(x_, 2.0);

  auto it_y = env_.find(y_);

  EXPECT_EQ(it_y, env_.end());
}

TEST_F(TestEnvironment, AccessNonExistingKey) {
  [[maybe_unused]] numeric_type val;
  EXPECT_THROW(val = env_.at(x_), std::out_of_range);
}

TEST_F(TestEnvironment, SizeAndEmpty) {
  EXPECT_TRUE(env_.empty());
  EXPECT_EQ(env_.size(), 0u);

  env_.insert(x_, 2.0);

  EXPECT_FALSE(env_.empty());
  EXPECT_EQ(env_.size(), 1u);

  env_.insert(y_, 3.0);

  EXPECT_EQ(env_.size(), 2u);
}

TEST_F(TestEnvironment, Domain) {
  env_.insert(x_, 2.0);
  env_.insert(y_, 3.0);

  Variables domain = env_.domain();

  EXPECT_EQ(domain.size(), 2u);
  EXPECT_TRUE(domain.include(x_));
  EXPECT_TRUE(domain.include(y_));
}

TEST_F(TestEnvironment, Stdout) {
  EXPECT_NO_THROW(std::cout << env_ << std::endl);
  env_.insert(x_, 2.0);
  EXPECT_NO_THROW(std::cout << env_ << std::endl);
  env_.insert(y_, 3.0);
  EXPECT_NO_THROW(std::cout << env_ << std::endl);
}