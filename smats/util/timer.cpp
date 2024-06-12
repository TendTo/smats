/**
 * @author Ernesto Casablanca (casablancaernesto@gmail.com)
 * @copyright 2024 smats
 * @licence Apache-2.0 license
 */

#include "smats/util/timer.h"

#include <sys/resource.h>

#include <stdexcept>

#include "smats/util/logging.h"

namespace smats {

template <class T>
TimerBase<T>::TimerBase() : last_start_{now()} {}

template <class T>
void TimerBase<T>::start() {
  SMATS_TRACE("TimerBase::start");
  last_start_ = now();
  elapsed_ = duration{0};
  running_ = true;
}

template <class T>
void TimerBase<T>::pause() {
  if (running_) {
    running_ = false;
    elapsed_ += (now() - last_start_);
  }
}

template <class T>
void TimerBase<T>::resume() {
  if (!running_) {
    last_start_ = now();
    running_ = true;
  }
}

template <class T>
bool TimerBase<T>::is_running() const {
  return running_;
}

template <class T>
typename TimerBase<T>::duration TimerBase<T>::elapsed() const {
  SMATS_TRACE("TimerBase::duration");
  return running_ ? elapsed_ + (now() - last_start_) : elapsed_;
}

template <class T>
std::chrono::duration<double>::rep TimerBase<T>::seconds() const {
  SMATS_TRACE("TimerBase::seconds");
  return std::chrono::duration_cast<std::chrono::duration<double>>(elapsed()).count();
}

user_clock::time_point user_clock::now() {
  SMATS_TRACE("user_clock::now");
  struct rusage usage {};
  if (0 != getrusage(RUSAGE_SELF, &usage)) throw std::runtime_error("Failed to get current resource usage (getrusage)");
  return time_point(duration(uint64_t(usage.ru_utime.tv_sec) * std::micro::den + uint64_t(usage.ru_utime.tv_usec)));
}

// Explicit instantiations
template class TimerBase<chosen_steady_clock>;
template class TimerBase<user_clock>;

TimerGuard::TimerGuard(Timer *const timer, const bool enabled, const bool start_timer)
    : timer_{timer}, enabled_{enabled && timer_ != nullptr} {
  if (enabled_ && start_timer) timer_->resume();
}

TimerGuard::~TimerGuard() {
  if (enabled_) timer_->pause();
}

void TimerGuard::pause() {
  if (enabled_) timer_->pause();
}

void TimerGuard::resume() {
  if (enabled_) timer_->resume();
}

}  // namespace smats
