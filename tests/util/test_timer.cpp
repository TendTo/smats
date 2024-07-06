/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */
#include <gtest/gtest.h>

#include "smats/util/timer.h"

using smats::Timer;
using smats::TimerGuard;
using smats::UserTimer;

void DoSomeWork(const int n) {
  [[maybe_unused]] int dummy = 0;
  [[maybe_unused]] std::string result;
  for (int i = 0; i < n; ++i) {
    dummy += i;
    result += std::to_string(dummy);
  }
}

TEST(TestTimer, TimerBehavior) {
  Timer timer;

  // Timer is not started.
  EXPECT_FALSE(timer.is_running());
  EXPECT_EQ(timer.elapsed(), Timer::clock::duration{0});

  // Start the timer.
  timer.start();
  DoSomeWork(1000);
  EXPECT_TRUE(timer.is_running());
  const auto duration1{timer.elapsed()};
  EXPECT_GT(duration1, Timer::clock::duration{0});

  // Pause the timer.
  timer.pause();
  EXPECT_FALSE(timer.is_running());
  const auto duration2{timer.elapsed()};
  DoSomeWork(1000);
  const auto duration3{timer.elapsed()};
  // Timer has been paused between duration2 and duration3.
  EXPECT_EQ(duration2, duration3);

  // Pause the timer.
  timer.resume();
  DoSomeWork(1000);
  const auto duration4{timer.elapsed()};
  EXPECT_LT(duration3, duration4);
  EXPECT_TRUE(timer.is_running());

  // Start the TestTimer this should reset it.
  timer.start();
  DoSomeWork(10);
  const auto duration5{timer.elapsed()};
  EXPECT_LE(duration5, duration1);
  EXPECT_TRUE(timer.is_running());
}

TEST(TestTimer, TimerGuardBehavior) {
  Timer timer;
  EXPECT_FALSE(timer.is_running());
  {
    TimerGuard timer_guard{&timer, true, true};
    EXPECT_TRUE(timer.is_running());
    DoSomeWork(1000);
    const auto duration = timer.elapsed();
    EXPECT_GT(duration, Timer::clock::duration{0});
  }
  EXPECT_FALSE(timer.is_running());
}

TEST(TestTimer, UserTimerBehavior) {
  UserTimer timer;

  // Timer is not started.
  EXPECT_FALSE(timer.is_running());
  EXPECT_EQ(timer.elapsed(), Timer::clock::duration{0});

  // Start the timer.
  timer.start();
  DoSomeWork(100000);
  EXPECT_TRUE(timer.is_running());
  const auto duration1{timer.elapsed()};
  EXPECT_GT(duration1, Timer::clock::duration{0});

  // Pause the timer.
  timer.pause();
  EXPECT_FALSE(timer.is_running());
  const auto duration2{timer.elapsed()};
  DoSomeWork(100000);
  const auto duration3{timer.elapsed()};
  // Timer has been paused between duration2 and duration3.
  EXPECT_EQ(duration2, duration3);

  // Pause the timer.
  timer.resume();
  DoSomeWork(100000);
  const auto duration4{timer.elapsed()};
  EXPECT_LT(duration3, duration4);
  EXPECT_TRUE(timer.is_running());

  // Start the TestTimer this should reset it.
  timer.start();
  DoSomeWork(10);
  const auto duration5{timer.elapsed()};
  EXPECT_LE(duration5, duration1);
  EXPECT_TRUE(timer.is_running());
}
