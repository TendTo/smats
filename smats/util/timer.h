/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Timer classes.
 *
 * The Timer class is a simple timer class to evaluate the performance of the software.
 * The TimerGuard class wraps a timer object and pauses it when the guard object is destructed.
 */
#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>
#include <type_traits>

namespace smats {

struct user_clock;

/**
 * Simple timer class to evaluate the performance of the software.
 */
template <typename T>
class TimerBase {
 public:
  using clock = T;
  typedef typename clock::duration duration;
  typedef typename clock::time_point time_point;

  /** @constructor{timer base} */
  TimerBase();

  /**
   * Start the timer.
   *
   * The timer is reset to zero.
   */
  void start();

  /**
   * Pause the timer.
   *
   * If the timer is not running, this function does nothing.
   */
  void pause();

  /**
   * Resume the timer.
   *
   * If the timer is not running, this function does nothing.
   */
  void resume();

  /** @checker{running, timer} */
  [[nodiscard]] bool is_running() const;
  /** @getter{duration of elapsed time, timer} */
  [[nodiscard]] duration elapsed() const;
  /** @getter{number elapsed seconds, timer} */
  [[nodiscard]] std::chrono::duration<double>::rep seconds() const;

  TimerBase<T> &operator+=(const TimerBase<T> &other);
  TimerBase<T> operator+(const TimerBase<T> &other) const;

 protected:
  /** @getter{current instant, timer} */
  [[nodiscard]] time_point now() const { return clock::now(); }

 private:
  bool running_{false};      ///< Whether the timer is running or not.
  time_point last_start_{};  ///< Last time_point when the timer is started or resumed.
  duration elapsed_{};       ///< Elapsed time so far. This doesn't include the current fragment if it is running.
};

// Use high_resolution clock if it's steady, otherwise use steady_clock.
using chosen_steady_clock = std::conditional<std::chrono::high_resolution_clock::is_steady,
                                             std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

extern template class TimerBase<chosen_steady_clock>;
class Timer : public TimerBase<chosen_steady_clock> {};

struct user_clock {  // Implements the Clock interface of std::chrono
  typedef uint64_t rep;
  typedef std::micro period;
  typedef std::chrono::duration<rep, period> duration;
  typedef std::chrono::time_point<user_clock> time_point;
  const bool is_steady = false;  // Not sure how this should be interpreted here
  static time_point now();
};

extern template class TimerBase<user_clock>;
class UserTimer : public TimerBase<user_clock> {};

/**
 * Wrap a timer object and pauses it when the guard object is destructed.
 */
class TimerGuard {
 public:
  /**
   * Construct a timer guard object.
   *
   * If @p enabled is false or @p timer is a nullptr, this class does not do anything.
   * If @p start_timer is true, starts the @p timer in the constructor.
   * Otherwise, it does not start it and a user has to call `resume()` to start it.
   * @param timer the timer to be guarded
   * @param enabled whether the timer is enabled and will run
   * @param start_timer whether the timer should be started as soon as the guard is created
   */
  TimerGuard(Timer *timer, bool enabled, bool start_timer = true);

  TimerGuard(const TimerGuard &) = delete;
  TimerGuard(TimerGuard &&) = delete;
  TimerGuard &operator=(const TimerGuard &) = delete;
  TimerGuard &operator=(TimerGuard &&) = delete;

  /**
   * When the timer guard object is destructed, it pauses the embedded timer object.
   * If the timer is not enabled, this function does nothing.
   */
  ~TimerGuard();

  /** Pause the guarded timer object */
  void pause();

  /** Resume the guarded timer object */
  void resume();

 private:
  Timer *const timer_;         ///< The timer to be guarded.
  const bool enabled_{false};  ///< Whether the timer is enabled.
};

}  // namespace smats
