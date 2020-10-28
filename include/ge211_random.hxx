#pragma once

#include "ge211_forward.hxx"
#include "ge211_error.hxx"

#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <type_traits>

struct Random_test_access;

namespace ge211 {

namespace detail {

template<class T>
struct Random_engine
{
    virtual T next() = 0;

    virtual ~Random_engine()
    { }
};

using std::enable_if_t;
using std::is_integral;
using std::is_floating_point;
using std::uniform_int_distribution;
using std::uniform_real_distribution;

template<class T, class D = void>
struct Distribution
{
};

template<class T>
struct Distribution<T, enable_if_t<is_integral<T>::value>>
        : uniform_int_distribution<T>
{
    Distribution(T begin, T end)
            : uniform_int_distribution<T>{begin, end - 1}
    { }
};

template<class T>
struct Distribution<T, enable_if_t<is_floating_point<T>::value>>
        : uniform_real_distribution<T>
{
    Distribution(T begin, T end)
            : uniform_real_distribution<T>{begin, end}
    { }
};

using Generator = std::mt19937_64;

Generator construct_generator();

template<class T>
class Pseudo_random_engine : public Random_engine<T>
{
public:
    Pseudo_random_engine(T min, T max)
            : distribution_{min, max}
            , generator_(construct_generator())
    { }

    T next() override
    {
        return distribution_(generator_);
    }

private:
    Distribution<T> distribution_;
    Generator       generator_;
};

template<>
class Pseudo_random_engine<bool> : public Random_engine<bool>
{
public:
    Pseudo_random_engine(double probability)
            : probability_{probability}
            , distribution_{0.0, 1.0}
            , generator_{construct_generator()}
    {
        if (probability_ < 0) {
            throw ge211::Client_logic_error{
                    "Random_source: probability cannot be < 0"};
        }

        if (probability_ > 1) {
            throw ge211::Client_logic_error{
                    "Random_source: probability cannot be > 1"};
        }
    }

    bool next() override
    {
        return probability_ > 0 &&
               distribution_(generator_) <= probability_;
    }

private:
    double               probability_;
    Distribution<double> distribution_;
    Generator            generator_;
};

template<class T>
class Stub_random_engine : public Random_engine<T>
{
public:
    // PRECONDITION: ! data.empty()
    Stub_random_engine(std::vector<T> data)
            : data_(std::move(data))
            , next_(0)
    { }

    T next() override
    {
        T result = data_[next_++];
        if (next_ == data_.size()) next_ = 0;
        return result;
    }

private:
    std::vector<T> data_;
    size_t         next_;
};

} // end namespace detail

/// A random number generator.
///
/// This class can be used to generate random numbers. To use it, first
/// you must instantiate it with a particular type, such as
/// `int` or `double`, and then provide the constructor with the minimum
/// and maximum values you would like to generate. For example, to generate
/// `int` values in the range from -7 to 7, you could write:
///
/// ```c
///     ge211::Random_source<int> my_int_source(-7, 7);
/// ```
///
/// Then call the member function Random_source::next to generate
/// a random value:
///
/// ```c
///     int a_random_int = my_int_source.next();
/// ```
///
/// For testing, you can “stub” your Random_source in order to
/// predetermine the sequence of values that it will return. See
/// Random_source::stub_with and Random_source::stub_with_vector.
template<class T>
class Random_source
{
    using Real = detail::Pseudo_random_engine<T>;
    using Stub = detail::Stub_random_engine<T>;

public:
    /// Constructs a random source that generates values between `min` and
    // `max`.
    Random_source(T min, T max)
            : engine_{std::make_unique<Real>(min, max)}
    { }

    /// Constructs a random source that generates values between `0` and
    /// `max - 1`.
    Random_source(T max)
            : Random_source{T(), max - 1}
    { }

    /// Returns the next random number from the random source.
    T next()
    {
        return engine_->next();
    }

    /// Returns the next random number from the random source. (This is an alias
    /// for Random_source::next().
    T operator()()
    {
        return next();
    }

    // Causes the random source to return the given values instead of using its
    // usual generation mechanism. If the values run out then the source will
    // repeat them as many times as necessary.
    void stub_with(std::initializer_list<T> values)
    {
        stub_with_vector(values);
    }

    // Causes the random source to return the values of the given vector instead
    // of using its usual generation mechanism. If the values run out then
    // the source will repeat them as many times as necessary.
    void stub_with_vector(std::vector<T> vec)
    {
        if (vec.empty()) {
            throw ge211::Client_logic_error{
                    "Random_source: cannot stub with empty vector"};
        }

        engine_ = std::make_unique<Stub>(std::move(vec));
    }

private:
    std::unique_ptr<detail::Random_engine<T>> engine_;
};

} // end namespace ge211
