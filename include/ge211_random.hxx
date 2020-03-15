#pragma once

#include "ge211_forward.hxx"
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

struct Random_test_access;

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

/// A pseudo-random number generator.
///
/// This class has member functions for generating random numbers.
///
/// Classes derived from Abstract_game can access an instance of
/// this class via Abstract_game::get_random(), which returns a
/// reference to a Random object maintained by Abstract_game. There
/// is no way for clients to construct their own instances of the
/// Random class.
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

    /// Returns a random `T` from the whole range of `T`.
    /// Only enabled for integral types `T`.
    template <
            class T,
            class = std::enable_if_t<std::is_integral<T>::value>
    >
    T any()
    {
        return between(std::numeric_limits<T>::min(),
                       std::numeric_limits<T>::max());
    }

    /// Returns a random `bool` that is `true` with probability
    /// `ptrue`.
    bool random_bool(double ptrue = 0.5);

    /// Can't copy the random number generator.
    Random(Random &) = delete;

    /// Can't copy the random number generator.
    Random& operator=(Random &) = delete;

    /// Can't move the random number generator.
    Random(Random &&) = delete;

    /// Can't move the random number generator.
    Random& operator=(Random &&) = delete;

private:
    // Creator:
    friend Abstract_game;

    // Random friend:
    friend Random_test_access;

    Random();

    std::mt19937_64 generator_;
};

}
