#pragma once

#include "ge211_forward.h"
#include <cstdint>
#include <random>
#include <type_traits>

namespace ge211 {

namespace detail {

template <class T, class Enable = void>
struct Between
{
    static_assert(
            std::is_integral<T>::value || std::is_floating_point<T>::value,
            "Random::between: only works on built-in numeric types"
    );
};

template <class T, class Enable = void>
struct Up_to {
    static_assert(
            std::is_integral<T>::value || std::is_floating_point<T>::value,
            "Random::up_to: only works on built-in numeric types"
    );
};

template <class T>
struct Between<T, std::enable_if_t<std::is_integral<T>::value>>
        : std::uniform_int_distribution<T>
{
    using std::uniform_int_distribution<T>::uniform_int_distribution;
};

template <class T>
struct Between<T, std::enable_if_t<std::is_floating_point<T>::value>>
        : std::uniform_real_distribution<T>
{
    using std::uniform_real_distribution<T>::uniform_real_distribution;
};

template <class T>
struct Up_to<T, std::enable_if_t<std::is_integral<T>::value>>
        : Between<T>
{
    explicit Up_to(T max) : Between<T>{0, max - 1}
    { }
};

template <class T>
struct Up_to<T, std::enable_if_t<std::is_floating_point<T>::value>>
        : Between<T>
{
    explicit Up_to(T max) : Between<T>{0, max}
    { }
};

} // end namespace detail

/// A pseudo-random number generator. This class has member functions
/// for generating random numbers.
class Random
{
public:
    /// Returns a random `T` between 0 (inclusive) and `max` (exclusive).
    ///
    /// Example:
    ///
    /// ```cpp
    /// int roll_the_die(Random& random)
    /// {
    ///       return random.up_to(6) + 1;
    /// }
    /// ```
    template <class T>
    T up_to(T max)
    {
        return detail::Up_to<T>{max}(generator_);
    }

    /// Returns a random `T` between `min` and `max`. The right bound
    /// is inclusive for integral types but exclusive for floating point
    /// types. The left bound is always inclusive.
    ///
    /// Example:
    ///
    /// ```cpp
    /// int roll_the_die(Random& random)
    /// {
    ///       return random.between(1, 6);
    /// }
    /// ```
    template <class T>
    T between(T min, T max)
    {
        return detail::Between<T>{min, max}(generator_);
    }

    /// Returns a random `bool` that is `true` with probability
    /// `ptrue`.
    bool random_bool(double ptrue = 0.5);

private:
    // Creator:
    friend Abstract_game;

    Random();

    Random(Random &) = delete;
    Random(Random &&) = delete;
    Random& operator=(Random &) = delete;
    Random& operator=(Random &&) = delete;

    std::mt19937_64 generator_;
};

}
