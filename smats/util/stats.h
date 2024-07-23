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

/**
 * Measure the statistics of the software.
 *
 * Namely, measure the time spent in operations.
 */
class Stats {
 public:
  /**
   * Construct a new Stats object.
   *
   * The @p enabled flag is used to enable or disable the stats measuring.
   * The @p class_name is the name of the class the stats are measuring,
   * while the @p name_time is the prompt to specify what operation the time has been measured of.
   * They are shown in the output of the to_string() method.
   * @param enabled whether the stats measuring is enabled
   * @param class_name name of the class the stats are measuring
   * @param name_time prompt to specify what operation the time has been measured of
   */
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
  [[nodiscard]] virtual std::string to_segment_string() const;
  /**
   * Convert the current state of the object to a formatted string
   * @return string representing the state of the Stats
   */
  [[nodiscard]] virtual std::string to_string() const;

 protected:
  bool enabled_;                 ///< Whether the stats measuring is enabled
  std::string class_name_;       ///< Name of the class the stats are measuring
  std::string operations_name_;  ///< Prompt to specify what operation the time has been measured of

 private:
  Timer timer_;  ///< Timer to measure the time spent in operations
};

/**
 * Measure the statistics of the software.
 *
 * Namely, measure the time spent in operations and the number of iterations.
 */
class IterationStats : public Stats {
 public:
  /**
   * Construct a new IterationStats object.
   *
   * The @p enabled flag is used to enable or disable the stats measuring.
   * The @p class_name is the name of the class the stats are measuring,
   * while the @p name_time is the prompt to specify what operation the time has been measured of,
   * and the @p iterations_name is the prompt to specify the number of iterations.
   * @param enabled whether the stats measuring is enabled
   * @param class_name name of the class the stats are measuring
   * @param name_time prompt to specify what operation the time has been measured of
   * @param iterations_name prompt to specify the number of iterations
   */
  explicit IterationStats(bool enabled, std::string class_name, std::string name_time = "Time spent in Operations",
                          std::string iterations_name = "Total # of Iterations");
  IterationStats(const IterationStats &other);
  IterationStats &operator=(const IterationStats &other);

  IterationStats &operator+=(const IterationStats &other);
  IterationStats operator+(const IterationStats &other) const;

  [[nodiscard]] std::string to_segment_string() const override;

  [[nodiscard]] std::string to_string() const override;

  /**
   * Increase the iteration counter by one.
   * @note The iteration counter is atomic.
   */
  void increase();

  [[nodiscard]] unsigned int iterations() const { return iterations_.load(); }

  void operator++();
  void operator++(int);

 private:
  std::atomic<unsigned int> iterations_;  ///< Number of iterations
  std::string iterations_name_;           ///< Prompt to specify the number of iterations
};

std::ostream &operator<<(std::ostream &os, const Stats &stats);
std::ostream &operator<<(std::ostream &os, const IterationStats &stats);

}  // namespace smats
