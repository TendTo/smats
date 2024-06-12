/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 * Stats class.
 * Used as a base class for more specialized stats classes.
 *
 * Simple dataclass to store the statistics of a solver run.
 */
#pragma once

#include <atomic>
#include <iostream>
#include <string>

#include "smats/util/timer.h"

namespace smats {

class Stats {
 private:
  Timer timer_;

 protected:
  bool enabled_;
  std::string class_name_;
  std::string operations_name_;

 public:
  explicit Stats(bool enabled, std::string class_name, std::string name_time = "Time spent in Operations");
  Stats(const Stats &other) = default;
  Stats &operator=(const Stats &other) = default;
  virtual ~Stats() = default;

  Stats &operator+=(const Stats &other);
  Stats operator+(const Stats &other) const;

  /** @checker{enabled, stats} */
  [[nodiscard]] bool enabled() const { return enabled_; }
  /** @getsetter{timer, stats} */
  [[nodiscard]] Timer &m_timer() { return timer_; }
  /** @getter{timer, stats} */
  [[nodiscard]] const Timer &timer() const { return timer_; }
  /**
   * Convert the current state of the object to a formatted string, only including the specific
   * part the Stat object is concerned about
   * @return string representing a partial state of the Stats
   */
  [[nodiscard]] virtual std::string ToSegmentString() const;
  /**
   * Convert the current state of the object to a formatted string
   * @return string representing the state of the Stats
   */
  [[nodiscard]] virtual std::string ToString() const;
};

class IterationStats : public Stats {
 private:
  std::atomic<unsigned int> iterations_;
  std::string iterations_name_;

 public:
  explicit IterationStats(bool enabled, std::string class_name, std::string name_time = "Time spent in Operations",
                          std::string iterations_name = "Total # of Iterations");
  IterationStats(const IterationStats &other);
  IterationStats &operator=(const IterationStats &other);

  IterationStats &operator+=(const IterationStats &other);
  IterationStats operator+(const IterationStats &other) const;

  [[nodiscard]] std::string ToSegmentString() const override;

  [[nodiscard]] std::string ToString() const override;

  /**
   * Increase the iteration counter by one.
   * @note The iteration counter is atomic.
   */
  void Increase();

  [[nodiscard]] unsigned int iterations() const { return iterations_.load(); }

  void operator++();
  void operator++(int);
};

std::ostream &operator<<(std::ostream &os, const Stats &stats);
std::ostream &operator<<(std::ostream &os, const IterationStats &stats);

}  // namespace smats
