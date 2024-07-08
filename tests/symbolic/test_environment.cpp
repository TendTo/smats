#include <gtest/gtest.h>

#include <array>
#include <utility>

#include "smats/symbolic/environment.h"

using smats::Environment;
using smats::Variable;
using smats::Variables;

template <class T>
class TestEnvironment : public ::testing::Test {
 protected:
  const Variable x_{"x"};
  const Variable y_{"y"};
  Environment<T> env_;
};

using TestParams = ::testing::Types<int, long, float, double>;
TYPED_TEST_SUITE(TestEnvironment, TestParams);

TYPED_TEST(TestEnvironment, KeyValueConstructors) {
  Environment<TypeParam> env_initializer_list{{this->x_, static_cast<TypeParam>(this->x_.id())},
                                              {this->y_, static_cast<TypeParam>(this->y_.id())}};
  Environment<TypeParam> env_vector(std::vector<std::pair<const Variable, TypeParam>>{
      {this->x_, static_cast<TypeParam>(this->x_.id())}, {this->y_, static_cast<TypeParam>(this->y_.id())}});
  Environment<TypeParam> env_array(
      std::array<std::pair<const Variable, TypeParam>, 2>{std::pair{this->x_, static_cast<TypeParam>(this->x_.id())},
                                                          std::pair{this->y_, static_cast<TypeParam>(this->y_.id())}});
  const std::pair<const Variable, TypeParam> arr[] = {{this->x_, static_cast<TypeParam>(this->x_.id())},
                                                      {this->y_, static_cast<TypeParam>(this->y_.id())}};
  Environment<TypeParam> env_arr(arr);

  EXPECT_EQ(env_initializer_list.size(), 2u);
  EXPECT_TRUE(env_initializer_list.contains(this->x_));
  EXPECT_TRUE(env_initializer_list.contains(this->y_));
  EXPECT_EQ(env_initializer_list.at(this->x_), static_cast<TypeParam>(this->x_.id()));
  EXPECT_EQ(env_initializer_list.at(this->y_), static_cast<TypeParam>(this->y_.id()));

  EXPECT_EQ(env_initializer_list, env_vector);
  EXPECT_EQ(env_initializer_list, env_array);
  EXPECT_EQ(env_initializer_list, env_arr);
}

TYPED_TEST(TestEnvironment, KeyConstructors) {
  Environment<TypeParam> env_initializer_list{this->x_, this->y_};
  Environment<TypeParam> env_vector(std::vector<Variable>{this->x_, this->y_});
  Environment<TypeParam> env_array(std::array<Variable, 2>{this->x_, this->y_});
  const Variable arr[] = {this->x_, this->y_};
  Environment<TypeParam> env_arr(arr);

  EXPECT_EQ(env_initializer_list.size(), 2u);
  EXPECT_TRUE(env_initializer_list.contains(this->x_));
  EXPECT_TRUE(env_initializer_list.contains(this->y_));
  EXPECT_EQ(env_initializer_list.at(this->x_), 0);
  EXPECT_EQ(env_initializer_list.at(this->y_), 0);

  EXPECT_EQ(env_initializer_list, env_vector);
  EXPECT_EQ(env_initializer_list, env_array);
  EXPECT_EQ(env_initializer_list, env_arr);
}

TYPED_TEST(TestEnvironment, InsertAndAccess) {
  this->env_.insert(this->x_, 2);
  this->env_.insert(this->y_, 3);

  EXPECT_EQ(this->env_.at(this->x_), 2);
  EXPECT_EQ(this->env_.at(this->y_), 3);
}

TYPED_TEST(TestEnvironment, InsertOrAssign) {
  this->env_.insert_or_assign(this->x_, 2);
  this->env_.insert_or_assign(this->y_, 3);
  this->env_.insert_or_assign(this->x_, 4);

  EXPECT_EQ(this->env_.at(this->x_), 4);
  EXPECT_EQ(this->env_.at(this->y_), 3);
}

TYPED_TEST(TestEnvironment, FindExistingKey) {
  this->env_.insert(this->x_, 2);
  this->env_.insert(this->y_, 3);

  auto it_x = this->env_.find(this->x_);
  auto it_y = this->env_.find(this->y_);

  EXPECT_NE(it_x, this->env_.end());
  EXPECT_NE(it_y, this->env_.end());
  EXPECT_EQ(it_x->second, 2);
  EXPECT_EQ(it_y->second, 3);
}

TYPED_TEST(TestEnvironment, FindNonExistingKey) {
  this->env_.insert(this->x_, 2);

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

  this->env_.insert(this->x_, 2);

  EXPECT_FALSE(this->env_.empty());
  EXPECT_EQ(this->env_.size(), 1u);

  this->env_.insert(this->y_, 3);

  EXPECT_EQ(this->env_.size(), 2u);
}

TYPED_TEST(TestEnvironment, Domain) {
  this->env_.insert(this->x_, 2);
  this->env_.insert(this->y_, 3);

  Variables domain = this->env_.domain();

  EXPECT_EQ(domain.size(), 2u);
  EXPECT_TRUE(domain.contains(this->x_));
  EXPECT_TRUE(domain.contains(this->y_));
}

TYPED_TEST(TestEnvironment, Stdout) {
  EXPECT_NO_THROW(std::cout << this->env_ << std::endl);
  this->env_.insert(this->x_, 2);
  EXPECT_NO_THROW(std::cout << this->env_ << std::endl);
  this->env_.insert(this->y_, 3);
  EXPECT_NO_THROW(std::cout << this->env_ << std::endl);
}