#pragma once

#include "forward.hxx"
#include "ge211/util/ring_buffer.hxx"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <ratio>
#include <thread>
#include <type_traits>
#include <utility>

namespace ge211 {

namespace detail {

using System_clock = std::conditional_t<
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
    using duration_type_ = detail::System_clock::duration;
    using seconds_type_ = std::chrono::duration<double>;
    using millis_type_ = std::chrono::duration<long, std::milli>;

    duration_type_ duration_;

    template <typename ToType, typename FromType>
    static ToType
    cast_(FromType v)
    {
        return std::chrono::duration_cast<ToType>(v);
    }

public:
    /// Constructs the zero duration.
    Duration()
            : duration_{}
    { }

    /// Constructs the duration of the given number of seconds.
    explicit Duration(double seconds)
            : Duration{seconds_type_{seconds}}
    { }

    /// Gets this duration in seconds.
    double seconds() const
    {
        auto s = cast_<seconds_type_>(duration_);
        return s.count();
    }

    /// Gets this duration, approximately, in milliseconds.
    long milliseconds() const
    {
        auto ms = cast_<millis_type_>(duration_);
        return ms.count();
    }

    /// \name Comparisons
    ///@{

    /// Does this Duration equal another one?
    friend inline bool operator==(Duration a, Duration b)
    {
        return a.duration_ == b.duration_;
    }

    /// Does this Duration NOT equal another one?
    friend inline bool operator!=(Duration a, Duration b)
    {
        return a.duration_ != b.duration_;
    }

    /// Less-than for Duration.
    friend inline bool operator<(Duration a, Duration b)
    {
        return a.duration_ < b.duration_;
    }

    /// Less-than-or-equal-to for Duration.
    friend inline bool operator<=(Duration a, Duration b)
    {
        return a.duration_ <= b.duration_;
    }

    /// Greater-than for Duration.
    friend inline bool operator>(Duration a, Duration b)
    {
        return a.duration_ > b.duration_;
    }

    /// Greater-than-or-equal-to for Duration.
    friend inline bool operator>=(Duration a, Duration b)
    {
        return a.duration_ >= b.duration_;
    }

    ///@}

    ///\name Arithmetic
    ///@{

    /// Addition for Duration.
    friend inline Duration operator+(Duration a, Duration b)
    {
        return {a.duration_ + b.duration_};
    }

    /// Subtraction for Duration.
    friend inline Duration operator-(Duration a, Duration b)
    {
        return {a.duration_ - b.duration_};
    }

    /// Multiplication for Duration.
    friend inline Duration operator*(Duration a, double factor)
    {
        return {a.duration_ * factor};
    }

    /// Multiplication for Duration.
    friend inline Duration operator*(double factor, Duration a)
    {
        return {a.duration_ * factor};
    }

    /// Division for Duration.
    friend inline Duration operator/(Duration a, double factor)
    {
        return {a.duration_ / factor};
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
    friend class Time_point;

    friend class detail::Engine;

    Duration(seconds_type_ duration)
            : Duration(cast_<duration_type_>(duration))
    { }

    Duration(duration_type_ duration)
            : duration_{duration}
    { }

    void sleep_for() const
    {
        std::this_thread::sleep_for(duration_);
    }
};

/// A point in time. Time_point values can be compared; they cannot
/// be added, but they can be subtracted to produce Duration values,
/// and they can be shifted by Duration values.
class Time_point
{
    using time_point_type_ = detail::System_clock::time_point;

    time_point_type_ time_point_;

    Time_point(time_point_type_ time_point)
            : time_point_{time_point}
    { }

public:
    /// Constructs the zero time point (the epoch).
    Time_point()
            : time_point_{}
    { }

    /// Returns the current time.
    static Time_point now()
    { return Time_point(detail::System_clock::now()); }

    /// \name Comparisons
    ///@{

    /// Equality for Time_point.
    friend inline bool operator==(Time_point a, Time_point b)
    {
        return a.time_point_ == b.time_point_;
    }

    /// Disequality for Time_point.
    friend inline bool operator!=(Time_point a, Time_point b)
    {
        return a.time_point_ != b.time_point_;
    }

    /// Is this Time_point earlier than that one?
    friend inline bool operator<(Time_point a, Time_point b)
    {
        return a.time_point_ < b.time_point_;
    }

    /// Is this Time_point earlier than or equal to that one?
    friend inline bool operator<=(Time_point a, Time_point b)
    {
        return a.time_point_ <= b.time_point_;
    }

    /// Is this Time_point later than that one?
    friend inline bool operator>(Time_point a, Time_point b)
    {
        return a.time_point_ > b.time_point_;
    }

    /// Is this Time_point later than or equal to that one?
    friend inline bool operator>=(Time_point a, Time_point b)
    {
        return a.time_point_ >= b.time_point_;
    }

    ///@}

    ///\name Arithmetic
    ///@{

    /// Finds the Duration between one Time_point and another.
    friend inline Duration operator-(Time_point a, Time_point b)
    {
        return Duration{a.time_point_ - b.time_point_};
    }

    /// Offsets a Time_point by adding a Duration.
    friend inline Time_point operator+(Time_point a, Duration b)
    {
        return Time_point{a.time_point_ + b.duration_};
    }

    /// Offsets a Time_point subtracting by a Duration.
    friend inline Time_point operator-(Time_point a, Duration b)
    {
        return Time_point{a.time_point_ - b.duration_};
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

        if (is_paused_) {
            elapsed_time_ = Duration{};
        } else {
            fake_start_time_ = now_();
        }
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
        Time_point fake_start_time_;    // when not paused
        Duration elapsed_time_;         // when paused
    };
    bool is_paused_;

    static Time_point now_()
    { return Time_point::now(); }
};

} // end namespace time

}  // end namespace ge211
