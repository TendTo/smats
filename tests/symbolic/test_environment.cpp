#include <gtest/gtest.h>

#include "smats/symbolic/environment.h"

using smats::Environment;
using smats::Variable;
using smats::Variables;

#define TEnvironment Environment<TypeParam>

template <class T>
class TestEnvironment : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  Environment<T> env_;
};

using TestParams = ::testing::Types<int, float, double, mpq_class>;
TYPED_TEST_SUITE(TestEnvironment, TestParams);

TYPED_TEST(TestEnvironment, InsertAndAccess) {
  this->env_.insert(this->x_, 2.0);
  this->env_.insert(this->y_, 3.0);

  EXPECT_EQ(this->env_.at(this->x_), 2.0);
  EXPECT_EQ(this->env_.at(this->y_), 3.0);
}

TYPED_TEST(TestEnvironment, InsertOrAssign) {
  this->env_.insert_or_assign(this->x_, 2.0);
  this->env_.insert_or_assign(this->y_, 3.0);
  this->env_.insert_or_assign(this->x_, 4.0);

  EXPECT_EQ(this->env_.at(this->x_), 4.0);
  EXPECT_EQ(this->env_.at(this->y_), 3.0);
}

TYPED_TEST(TestEnvironment, FindExistingKey) {
  this->env_.insert(this->x_, 2.0);
  this->env_.insert(this->y_, 3.0);

  auto it_x = this->env_.find(this->x_);
  auto it_y = this->env_.find(this->y_);

  EXPECT_NE(it_x, this->env_.end());
  EXPECT_NE(it_y, this->env_.end());
  EXPECT_EQ(it_x->second, 2.0);
  EXPECT_EQ(it_y->second, 3.0);
}

TYPED_TEST(TestEnvironment, FindNonExistingKey) {
  this->env_.insert(this->x_, 2.0);

  auto it_y = this->env_.find(this->y_);

  EXPECT_EQ(it_y, this->env_.end());
}

TYPED_TEST(TestEnvironment, AccessNonExistingKey) {
  [[maybe_unused]] TypeParam val;
  EXPECT_THROW(val = this->env_.at(this->x_), std::out_of_range);
}

TYPED_TEST(TestEnvironment, SizeAndEmpty) {
  EXPECT_TRUE(this->env_.empty());
  EXPECT_EQ(this->env_.size(), 0u);

  this->env_.insert(this->x_, 2.0);

  EXPECT_FALSE(this->env_.empty());
  EXPECT_EQ(this->env_.size(), 1u);

  this->env_.insert(this->y_, 3.0);

  EXPECT_EQ(this->env_.size(), 2u);
}

TYPED_TEST(TestEnvironment, Domain) {
  this->env_.insert(this->x_, 2.0);
  this->env_.insert(this->y_, 3.0);

  Variables domain = this->env_.domain();

  EXPECT_EQ(domain.size(), 2u);
  EXPECT_TRUE(domain.include(this->x_));
  EXPECT_TRUE(domain.include(this->y_));
}

TYPED_TEST(TestEnvironment, Stdout) {
  EXPECT_NO_THROW(std::cout << this->env_ << std::endl);
  this->env_.insert(this->x_, 2.0);
  EXPECT_NO_THROW(std::cout << this->env_ << std::endl);
  this->env_.insert(this->y_, 3.0);
  EXPECT_NO_THROW(std::cout << this->env_ << std::endl);
}