/**
 * @author smats (https://github.com/TendTo/smats)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "smats/util/hash.hpp"

using namespace smats;

class MockHashAlgorithm {
 public:
  using result_type = size_t;
  static const size_t default_;

  MOCK_METHOD2(Invoke, void(const void*, size_t));
  MOCK_METHOD0(Invoke, size_t(void));

  void operator()(const void* data, size_t length) noexcept { Invoke(data, length); }
  explicit operator size_t() noexcept {
    Invoke();
    return default_;
  }
};
const size_t MockHashAlgorithm::default_ = 1;

class TestHash : public ::testing::Test {
 protected:
  const size_t hash_ = 10203658981158674303ul;
  MockHashAlgorithm hasher_;
};

TEST_F(TestHash, HashAppendIntegral) {
  int value = 42;
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(int))).Times(1);
  hash_append(hasher_, value);
}

TEST_F(TestHash, HashAppendPointer) {
  int* ptr = nullptr;
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(uintptr_t))).Times(1);
  hash_append(hasher_, ptr);
}

TEST_F(TestHash, HashAppendEnum) {
  enum class Color { RED, GREEN, BLUE };
  Color color = Color::GREEN;
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(Color))).Times(1);
  hash_append(hasher_, color);
}

TEST_F(TestHash, HashAppendFloatingPoint) {
  double value = 3.14;
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(double))).Times(1);
  hash_append(hasher_, value);
}

TEST_F(TestHash, HashAppendString) {
  const std::string str = "Hello, world!";
  EXPECT_CALL(hasher_, Invoke(testing::_, str.size())).Times(1);
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(size_t))).Times(1);
  hash_append(hasher_, str);
}

TEST_F(TestHash, HashAppendPair) {
  std::pair<int, std::string> pair = {42, "Hello"};
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(int))).Times(1);
  EXPECT_CALL(hasher_, Invoke(testing::_, pair.second.size())).Times(1);
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(size_t))).Times(1);
  hash_append(hasher_, pair);
}

TEST_F(TestHash, HashAppendOptional) {
  std::optional<int> opt = 42;
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(int))).Times(1);
  EXPECT_CALL(hasher_, Invoke(testing::_, sizeof(bool))).Times(1);
  hash_append(hasher_, opt);
}

TEST_F(TestHash, HashAppendMap) {
  std::map<int, double> map = {{1, 2.1}, {2, 3.1}, {3, 4.1}};
  EXPECT_CALL(hasher_, Invoke(testing::_, testing::_)).Times(3 * 2 + 1);
  hash_append(hasher_, map);
}

TEST_F(TestHash, HashAppendSet) {
  std::set<int> set = {1, 2, 3};
  EXPECT_CALL(hasher_, Invoke(testing::_, testing::_)).Times(3 + 1);
  hash_append(hasher_, set);
}

TEST_F(TestHash, UHash) {
  std::string str = "Hello, world!";
  uhash<MockHashAlgorithm> uhasher;
  EXPECT_EQ(uhasher(str), MockHashAlgorithm::default_);
}

TEST_F(TestHash, DefaultHashAlgorithm) {
  DefaultHashAlgorithm hasher;
  const int value = 42;
  hasher(&value, sizeof(int));
  EXPECT_EQ(static_cast<size_t>(hasher), hash_);
}

TEST_F(TestHash, DefaultHash) {
  DefaultHash hasher;
  const int value = 42;
  EXPECT_EQ(hasher(value), hash_);
}