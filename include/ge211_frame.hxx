#pragma once

#include "ge211_forward.hxx"
#include "ge211_time.hxx"

namespace ge211 {

namespace detail {

template <
        size_t Sample_Period = 10, // input smoothing
        size_t Buffer_Size = 8     // output smoothing
>
class Perf_clock
{
public:
    void begin_compute_frame();
    void end_compute_frame();

    double frame_rate() const;
    double load_fraction() const;

private:
    static constexpr double decay_rate_ = 0.5;

    Pausable_timer busy_timer_;
    Timer real_timer_;

    size_t frame_count_{};
    Ring_buffer<Duration, Buffer_Size> samples_;
    Duration busy_decay_sum_;
    Duration real_decay_sum_;
    Duration real_sum_;

    double fps_ = 0;
    double load_ = 0;
};


class Frame_clock
{
public:
    explicit Frame_clock(Duration expected_frame_length = Duration(0));

    void mark_frame();
    void mark_frame(Time_point now);
    void mark_present();

    Time_point frame_start_time() const;
    Duration prev_frame_length() const;

    double frame_rate() const;
    double load_fraction() const;

private:
    Time_point frame_start_;
    Duration prev_length_;
    Perf_clock<> perf_clock_;
};


/*
 * Perf_clock member functions
 */

template <size_t Sample_period, size_t Buffer_size>
void
Perf_clock<Sample_period, Buffer_size>::begin_compute_frame()
{
    busy_timer_.resume();
}

template <size_t Sample_period, size_t Buffer_size>
void
Perf_clock<Sample_period, Buffer_size>::end_compute_frame()
{
    busy_timer_.pause();

    if (++frame_count_ != Sample_period) { return; }

    frame_count_ = 0;

    auto busy_sample = busy_timer_.reset();
    auto real_sample = real_timer_.reset();

    busy_decay_sum_
            = decay_rate_ * (busy_decay_sum_ - busy_sample) + busy_sample;
    real_decay_sum_
            = decay_rate_ * (real_decay_sum_ - real_sample) + real_sample;
    real_sum_
            += real_sample - samples_.rotate(real_sample);

    fps_ = Sample_period * samples_.size() / real_sum_.seconds();
    load_ = busy_decay_sum_.seconds() / real_decay_sum_.seconds();
}

template <size_t Sample_period, size_t Buffer_size>
double
Perf_clock<Sample_period, Buffer_size>::frame_rate() const
{
    return fps_;
}

template <size_t Sample_period, size_t Buffer_size>
double
Perf_clock<Sample_period, Buffer_size>::load_fraction() const
{
    return load_;
}


/*
 * Frame_clock member functions
 */

inline
Frame_clock::Frame_clock(Duration expected_frame_length)
        : frame_start_(Time_point::now()),
          prev_length_(expected_frame_length)
{ }

inline void
Frame_clock::mark_frame()
{
    mark_frame(Time_point::now());
}

inline void
Frame_clock::mark_frame(Time_point now)
{
    perf_clock_.begin_compute_frame();
    prev_length_ = now - frame_start_;
    frame_start_ = now;
}

inline void
Frame_clock::mark_present()
{
    perf_clock_.end_compute_frame();
}

inline Time_point
Frame_clock::frame_start_time() const
{
    return frame_start_;
}

inline Duration
Frame_clock::prev_frame_length() const
{
    return prev_length_;
}

inline double
Frame_clock::frame_rate() const
{
    return perf_clock_.frame_rate();
}

inline double
Frame_clock::load_fraction() const
{
    return perf_clock_.load_fraction();
}

}  // end namespace detail

}  // end namespace ge211
