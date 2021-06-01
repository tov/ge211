#pragma once

#include "ge211_forward.hxx"

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
    using chrono_type = std::chrono::duration<double>;

public:
    /// Constructs the zero duration.
    Duration() : duration_{} {}

    /// Constructs the duration of the given number of seconds.
    explicit Duration(double seconds)
            : Duration{chrono_type{seconds}} {}

    /// Gets this duration in seconds.
    double seconds() const
    {
        auto s = std::chrono::duration_cast<chrono_type>(duration_);
        return s.count();
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
    friend Time_point;
    friend class detail::Engine;

    Duration(chrono_type duration)
            : Duration{std::chrono::duration_cast<detail::System_clock::duration>
                               (duration)} {}

    Duration(detail::System_clock::duration duration)
            : duration_{duration} {}

    void sleep_for() const
    {
        std::this_thread::sleep_for(duration_);
    }

    detail::System_clock::duration duration_;
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
    static Time_point now() { return Time_point(detail::System_clock::now()); }

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

private:
    Time_point(detail::System_clock::time_point time_point)
            : time_point_{time_point} {}

    detail::System_clock::time_point time_point_;
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

namespace detail {

template <
    typename Value,
    unsigned N,
    typename Self
>
class Modulo_kernel {
public:
    using value_type = Value;

    explicit constexpr Modulo_kernel(Value value)
            : value_(value % N)
    { }

    constexpr operator Value() const
    { return value_; }

    Self&
    operator++()
    {
        value_ = (value_ + 1) % N;
        return *this;
    }

    Self&
    operator--()
    {
        value_ = (value_ - 1) % N;
        return *this;
    }

private:
    Value value_;

    static_assert(
            std::is_unsigned<value_type>::value,
            "Modulo requires value_type to be unsigned"
    );
};

template <
    typename Value,
    unsigned N
>
struct Modulo :
    Modulo_kernel<
        Value,
        N,
        Modulo<Value, N>
    >
{
    using Base = Modulo_kernel<Value, N, Modulo<Value, N>>;
    using typename Base::value_type;
    using Base::operator Value;
    using Base::operator++;
    using Base::operator--;

    constexpr Modulo() : Modulo(value_type()) { }
    constexpr Modulo(value_type value) : Base(value) { }
    constexpr Modulo(Modulo const&) = default;
    constexpr Modulo(Modulo&&) = default;
    constexpr Modulo& operator=(Modulo const&) = default;
    constexpr Modulo& operator=(Modulo&&) = default;

    constexpr Modulo
    operator++(int)
    {
        Modulo copy(*this);
        ++*this;
        return copy;
    }

    constexpr Modulo
    operator--(int)
    {
        Modulo copy(*this);
        --*this;
        return copy;
    }
};

template <typename Element, std::size_t Capacity>
class Ring_buffer
{
public:
    using value_type = Element;

    static constexpr std::size_t capacity = Capacity;

    constexpr Ring_buffer() : Ring_buffer(Element())
    { }

    explicit constexpr Ring_buffer(Element fill)
    {
        buf_.fill(fill);
    }

private:
    using Buffer_  = std::array<value_type, capacity>;

public:
    constexpr bool
    empty() const
    {
        return size_ == 0;
    }

    constexpr bool
    full() const
    {
        return size_ == capacity;
    }

    constexpr bool
    rotate(value_type in, value_type& out)
    {
        if (full()) {
            out = std::exchange(buf_[start_++], in);
            start_ %= capacity;
            return true;
        } else {
            buf_[size_++] = in;
            return false;
        }
    }

    constexpr std::size_t
    offset() const
    {
        return start_;
    }

private:
    Buffer_ buf_;
    std::size_t start_{0}, size_{0};
};

template <
        std::size_t N,
        typename Base = double
>
struct Pow;

template <std::size_t N, typename Base = double>
constexpr auto
pow(Base base)
{
    return Pow<N, Base>::value(base);
}

template <typename Base>
struct Pow<0, Base>
{
    static constexpr double value(Base)
    {
        return 1;
    }
};

template <typename Base>
struct Pow<1, Base>
{
    static constexpr double value(Base base)
    {
        return double(base);
    }
};

template <std::size_t N, typename Base>
struct Pow
{
    static constexpr double value(Base base)
    {
        if (N & 1) {
            return base * pow<N - 1>(base);
        } else {
            return pow<(N >> 1)>(base * base);
        }
    }
};

template <
    // input smoothing:
    std::size_t Lg_Smoothing = 6,
    // output smoothing:
    std::size_t Sample_Size = 10
>
class Perf_clock
{
public:
    static constexpr std::size_t N = 1 << Lg_Smoothing;

private:
    struct Sample
    {
        Duration busy;
        Duration real;
    };

    static constexpr double decay_rate_ = 0.5;
    static constexpr double decay_ = pow<N>(decay_rate_);
    static constexpr double decay_rate_sum_ = 1 / (1 - decay_rate_);

public:
    void begin_compute_frame()
    {
        busy_timer_.resume();
    }

    void end_compute_frame()
    {
        busy_timer_.pause();

        if (++frame_count_ != Sample_Size) return;

        frame_count_ = 0;

        auto busy_sample = busy_timer_.reset();
        auto real_sample = real_timer_.reset();

        busy_sum_ = decay_rate_ * busy_sum_ + busy_sample;
        real_sum_ = decay_rate_ * real_sum_ + real_sample;

        Sample oldest;
        if (samples_.rotate({busy_sample, real_sample}, oldest)) {
            busy_sum_ -= oldest.busy * decay_;
            real_sum_ -= oldest.real * decay_;

            fps_  = Sample_Size * N * decay_rate_sum_ / real_sum_.seconds();
            load_ = busy_sum_.seconds() / real_sum_.seconds();
        }
    }

    double fps() const
    { return fps_; }

    double load() const
    { return load_; }

private:
    Pausable_timer  busy_timer_;
    Timer           real_timer_;
    std::size_t     frame_count_{};
    Ring_buffer<Sample, N> samples_;
    Duration        busy_sum_, real_sum_;
    double          fps_ = 0;
    double          load_ = 0;
};

class Frame_clock
{
public:
    Frame_clock(Duration expected_frame_length = Duration(0))
            : frame_start_(Time_point::now()),
              prev_length_(expected_frame_length)
    { }

    void mark_frame()
    {
        mark_frame(Time_point::now());
    }

    void mark_frame(Time_point now)
    {
        perf_clock_.begin_compute_frame();
        prev_length_ = now - frame_start_;
        frame_start_ = now;
    }

    void mark_present()
    {
        perf_clock_.end_compute_frame();
    }

    Time_point frame_start_time() const
    {
        return frame_start_;
    }

    Duration prev_frame_length() const
    {
        return prev_length_;
    }

private:
    Time_point          frame_start_;
    Duration            prev_length_;
    Perf_clock<>        perf_clock_;
};


} // end namespace detail

}

