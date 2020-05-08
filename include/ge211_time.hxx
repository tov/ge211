#pragma once

#include "ge211_forward.hxx"

#include <chrono>
#include <ratio>
#include <thread>

namespace ge211 {

namespace detail {

using Clock = std::conditional_t<
        std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock,
        std::chrono::steady_clock>;

} // end namespace detail

/// Types for representing time and timers.
namespace time {

/// A length of time. This is an opaque type representing
/// a high-precision segment of time that can be extracted
/// as a `double` of seconds by the
/// seconds() const member function.
/// A Duration can be constructed from the number of seconds,
/// also as a `double`. Duration values can also be compared,
/// added, subtracted, and added and subtracted to Time_point values.
class Duration
{
public:
    /// Constructs the zero duration.
    Duration() : duration_{} {}

    /// Constructs the duration of the given number of seconds.
    explicit Duration(double seconds)
            : Duration{std::chrono::duration<double>{seconds}} {}

    /// Gets this duration in seconds.
    double seconds() const
    {
        auto seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                        duration_);
        return seconds.count();
    }

    /// Gets this duration, approximately, in milliseconds.
    long milliseconds() const
    {
        auto millis =
                std::chrono::duration_cast<std::chrono::duration<long, std::milli>>(
                        duration_);
        return millis.count();
    }

    /// \name Comparisons
    ///@{

    /// Does this Duration equal another one?
    bool operator==(Duration other) const
    {
        return duration_ == other.duration_;
    }

    /// Does this Duration NOT equal another one?
    bool operator!=(Duration other) const
    {
        return duration_ != other.duration_;
    }

    /// Less-than for Duration.
    bool operator<(Duration other) const
    {
        return duration_ < other.duration_;
    }

    /// Less-than-or-equal-to for Duration.
    bool operator<=(Duration other) const
    {
        return duration_ <= other.duration_;
    }

    /// Greater-than for Duration.
    bool operator>(Duration other) const
    {
        return duration_ > other.duration_;
    }

    /// Greater-than-or-equal-to for Duration.
    bool operator>=(Duration other) const
    {
        return duration_ >= other.duration_;
    }

    ///@}

    ///\name Arithmetic
    ///@{

    /// Addition for Duration.
    Duration operator+(Duration other) const
    {
        return {duration_ + other.duration_};
    }

    /// Subtraction for Duration.
    Duration operator-(Duration other) const
    {
        return {duration_ - other.duration_};
    }

    /// Multiplication for Duration.
    Duration operator*(double factor) const
    {
        return {duration_ * factor};
    }

    /// Division for Duration.
    Duration operator/(double factor) const
    {
        return {duration_ / factor};
    }

    /// Addition for Duration.
    Duration& operator+=(Duration other)
    {
        return *this = *this + other;
    }

    /// Subtraction for Duration.
    Duration& operator-=(Duration other)
    {
        return *this = *this - other;
    }

    /// Multiplication for Duration.
    Duration& operator*=(double factor)
    {
        return *this = *this * factor;
    }

    /// Division for Duration.
    Duration& operator/=(double factor)
    {
        return *this = *this / factor;
    }

    ///@}

private:
    friend Time_point;
    friend class detail::Engine;

    Duration(std::chrono::duration<double> duration)
            : Duration{std::chrono::duration_cast<detail::Clock::duration>
                               (duration)} {}

    Duration(detail::Clock::duration duration)
            : duration_{duration} {}

    void sleep_for() const
    {
        std::this_thread::sleep_for(duration_);
    }

    detail::Clock::duration duration_;
};

/// A point in time. Time_point values can be compared; they cannot
/// be added, but they can be subtracted to produce Duration values,
/// and they can be shifted by Duration values.
class Time_point
{
public:
    /// Constructs the zero time point (the epoch).
    Time_point() : time_point_{} {}

    /// Returns the current time.
    static Time_point now() { return Time_point(detail::Clock::now()); }

    /// \name Comparisons
    ///@{

    /// Equality for Time_point.
    bool operator==(Time_point other) const
    {
        return time_point_ == other.time_point_;
    }

    /// Disequality for Time_point.
    bool operator!=(Time_point other) const
    {
        return time_point_ != other.time_point_;
    }

    /// Is this Time_point earlier than that one?
    bool operator<(Time_point other) const
    {
        return time_point_ < other.time_point_;
    }

    /// Is this Time_point earlier than or equal to that one?
    bool operator<=(Time_point other) const
    {
        return time_point_ <= other.time_point_;
    }

    /// Is this Time_point later than that one?
    bool operator>(Time_point other) const
    {
        return time_point_ > other.time_point_;
    }

    /// Is this Time_point later than or equal to that one?
    bool operator>=(Time_point other) const
    {
        return time_point_ >= other.time_point_;
    }

    ///@}

    ///\name Arithmetic
    ///@{

    /// Finds the Duration between one Time_point and another.
    Duration operator-(Time_point other) const
    {
        return Duration{time_point_ - other.time_point_};
    }

    /// Offsets a Time_point by adding a Duration.
    Time_point operator+(Duration duration) const
    {
        return Time_point{time_point_ + duration.duration_};
    }

    /// Offsets a Time_point subtracting by a Duration.
    Time_point operator-(Duration duration) const
    {
        return Time_point{time_point_ - duration.duration_};
    }

    /// Offsets a Time_point by adding on a Duration.
    Time_point& operator+=(Duration duration)
    {
        return *this = *this + duration;
    }

    /// Offsets a Time_point subtracting off a Duration.
    Time_point& operator-=(Duration duration)
    {
        return *this = *this - duration;
    }

    ///@}

private:
    Time_point(detail::Clock::time_point time_point)
            : time_point_{time_point} {}

    detail::Clock::time_point time_point_;
};

/// A class for timing intervals. The result is a Duration.
class Timer
{
public:
    /// Creates a new timer, running from the time it was created.
    Timer() : start_time_{now_()} {}

    /// Creates a timer whose “start time” is some Duration in the future.
    /// Suppose we want to wait 30 seconds and then do something. We could
    /// create a Timer whose “start time” is in 30 seconds, and then poll it
    /// on each frame. When it returns a non-negative number for its
    /// elapsed_time() const, we know that the time as arrived.
    static Timer future(Duration duration)
    {
        Timer result;
        result.start_time_ += duration;
        return result;
    }

    /// Resets a timer, returning the time it was at before it was reset.
    Duration reset()
    {
        Time_point previous = start_time_;
        start_time_ = now_();
        return start_time_ - previous;
    }

    /** Returns the actual time when this timer was started or most recently
      * reset.
      */
    Time_point start_time() const
    {
        return start_time_;
    }

    /** Returns how much time has elapsed since this timer was started or
      * most recently reset.
      */
    Duration elapsed_time() const
    {
        return now_() - start_time_;
    }

private:
    Time_point start_time_;

    static Time_point now_() { return Time_point::now(); }
};

/// A class for timing intervals while supporting pausing.
class Pausable_timer
{
public:
    /// Constructs a new pausable timer. The timer is started running by
    /// default, but can be started paused by passing `true`.
    explicit Pausable_timer(bool start_paused = false)
    {
        is_paused_ = start_paused;

        if (is_paused_)
            elapsed_time_ = Duration{};
        else
            fake_start_time_ = now_();
    }

    /// Checks whether the timer is currently paused.
    bool is_paused() const
    {
        return is_paused_;
    }

    /** The elapsed time since the start or most recent reset, not counting
      * paused times.
      */
    Duration elapsed_time() const
    {
        if (is_paused_) {
            return elapsed_time_;
        } else {
            return now_() - fake_start_time_;
        }
    }

    /// Pauses the timer. If the timer is already paused, has no effect. In
    /// either case, the elapsed time thus far is saved, and can be queried
    /// with elapsed_time() const, or will continue to accumulate if we
    /// unpause().
    Duration pause()
    {
        if (!is_paused_) {
            elapsed_time_ = now_() - fake_start_time_;
            is_paused_ = true;
        }

        return elapsed_time_;
    }

    /// Unpauses the timer. If the timer is already running, has no effect.
    void resume()
    {
        if (is_paused_) {
            fake_start_time_ = now_() - elapsed_time_;
            is_paused_ = false;
        }
    }

    /// Resets the timer, returning the elapsed time since starting or the
    /// most recent reset(). Leaves the pause state unchanged.
    Duration reset()
    {
        if (is_paused_) {
            auto result = elapsed_time_;
            elapsed_time_ = Duration{};
            return result;
        } else {
            auto now = now_();
            auto result = now - fake_start_time_;
            fake_start_time_ = now;
            return result;
        }
    }

private:
    union
    {
        Time_point fake_start_time_;        // when not paused
        Duration elapsed_time_; // when paused
    };
    bool is_paused_;

    static Time_point now_() { return Time_point::now(); }
};

} // end namespace time

}

