#pragma once

#include "ge211_error.hxx"
#include "ge211_if_cpp.hxx"
#include "ge211_forward.hxx"
#include "ge211_type_traits.hxx"

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <type_traits>

namespace ge211 {
namespace detail {
namespace random {

using Generator = std::mt19937_64;

Generator
construct_generator();

template <typename T>
T
bound_between(T value, T lo, T hi)
{
    return std::max(lo, std::min(hi, value));
}

template <typename RESULT_TYPE>
struct Random_engine
{
    using result_type = RESULT_TYPE;

    virtual result_type next() = 0;

    virtual result_type next_between(result_type, result_type) = 0;

    virtual ~Random_engine() = default;
};

template <typename RESULT_TYPE, typename ENABLE = void>
struct Distribution
{
    struct
    {
        RESULT_TYPE contents;
    } error = "ge211::Random_source<RESULT_TYPE> requires "
              "RESULT_TYPE to be a built-in arithmetic type like int "
              "or double.";
};

template <typename RESULT_TYPE>
class Distribution<
        RESULT_TYPE,
        std::enable_if_t<Is_Integral<RESULT_TYPE>>
>
{
public:
    using result_type = RESULT_TYPE;

private:
    using impl_type = std::uniform_int_distribution<result_type>;
    impl_type impl_;

public:
    Distribution(result_type lo, result_type hi)
            : impl_{lo, hi}
    { }

    explicit Distribution(result_type end)
            : impl_{0, end - 1}
    { }

    result_type operator()(Generator &gen)
    {
        return impl_(gen);
    }
};


template <typename RESULT_TYPE>
class Distribution<
        RESULT_TYPE,
        std::enable_if_t<Is_Floating_Point<RESULT_TYPE>>>
{
public:
    using result_type = RESULT_TYPE;

private:
    using impl_type = std::uniform_real_distribution<result_type>;
    impl_type impl_;

public:
    Distribution(result_type lo, result_type hi)
            : impl_{lo, hi}
    { }

    result_type operator()(Generator &gen)
    {
        return impl_(gen);
    }
};

template <typename RESULT_TYPE>
class Pseudo_random_engine
        : public Random_engine<RESULT_TYPE>
{
public:
    using result_type = RESULT_TYPE;

    template <typename... Args>
    Pseudo_random_engine(Args&&... args);

    result_type next() override;

    result_type next_between(result_type, result_type) override;

private:
    Distribution<result_type> distribution_;
    Generator generator_;
};

template <>
class Pseudo_random_engine<bool>
        : public Random_engine<bool>
{
public:
    explicit Pseudo_random_engine(double p_true);

    bool next() override;

    bool next_between(bool, bool) override;

private:
    double probability_;
    Distribution<double> distribution_;
    Generator generator_;
};

template <typename RESULT_TYPE>
class Stub_random_engine
        : public Random_engine<RESULT_TYPE>
{
public:
    using result_type = RESULT_TYPE;
    using container_type = std::vector<RESULT_TYPE>;
    using iterator_type = typename container_type::const_iterator;

    // PRECONDITION: !container.empty()
    Stub_random_engine(container_type&& container);

    result_type next() override;

    result_type next_between(result_type, result_type) override;

private:
    container_type container_;
    iterator_type next_;
};

}  // end namespace random
}  // end namespace detail

/// The type of special tag value @ref unbounded. Don’t construct this
/// yourself; just use @ref ge211::unbounded.
class unbounded_type
{ };

/// A tag value for passing to the constructor
/// @ref Random_source::Random_source(unbounded_type)
/// in order to delay specifying the range of numbers at construction
/// time. Instead, the range of numbers must be passed to
/// @ref Random_source::operator()(result_type, result_type)
/// or
/// @ref Random_source::next_between(result_type, result_type)
/// for each number you generate.
///
/// For an example, see @ref Random_source::Random_source(unbounded_type).
constexpr unbounded_type const unbounded { };

/// A generic class for generating [pseudorandom numbers] in uniform
/// distribution over a specified range.
///
/// For example, a @ref Random_source<float> generates `float`s, and a
/// @ref Random_source<int> generates `int`s. To specify a range, pass
/// the bounds to either of the two constructors,
/// @ref Random_source(result_type, result_type) or
/// @ref Random_source(result_type).
/// Then call @ref Random_source::next() on your @ref Random_source to
/// generate a random number.
///
/// There are also two constructors available only for particular result types:
///
///   - @ref Random_source(result_type limit) is defined only when result_type
///     is an integral type. It constructs a source that generates numbers from
///     0 up to, but excluding, `limit`.
///
///   - @ref Random_source(double p_true) is defined only when result_type
///     is `bool`. It takes a probability and generates `true` with that
///     probability and `false` otherwise.
///
/// ### Testing
///
/// You can *stub* your @ref Random_source in order to predetermine the
/// sequence of values that it will return. For details, see
/// @ref Random_source::stub_with(std::initializer_list<result_type>)
/// and
/// @ref Random_source::stub_with(std::vector<result_type>).
///
/// [pseudorandom numbers]:
///     <https://en.wikipedia.org/wiki/Pseudorandom_number_generator>
template <typename RESULT_TYPE>
class Random_source
{
public:
    /// The type of value generated by this @ref Random_source.
    using result_type = RESULT_TYPE;

    /// Constructs a random source that generates values between `lo` and
    /// `hi`, inclusive.
    ///
    /// Not defined when @ref result_type is `bool`. See
    /// @ref Random_source<bool>::Random_source(double) instead.
    ///
    /// \example
    ///
    /// ```cxx
    /// // Initialize the source to produce `int`s from 1 to 6:
    /// ge211::Random_source<int> six_sided_die(1, 6);
    ///
    /// // Generate a random roll:
    /// int roll_value = six_sided_die.next();
    ///
    /// while (roll_value != 6) {
    ///     std::cout << "You rolled " << roll_value << "; try again.\n";
    ///     roll_value = six_sided_die.next();
    /// }
    ///
    /// std::cout << "Finally rolled a 6!\n";
    /// ```
    IF_COMPILER(DECLARE_IF(!Is_Same<result_type, bool>))
    Random_source(result_type lo, result_type hi);


    /// Constructs, for integral @ref result_type%s only, a random source that
    /// generates values between `0` and `limit - 1`.
    ///
    /// Thus, `Random_source(limit)` is equivalent to
    /// [Random_source][1]`(0, limit - 1)`.
    ///
    /// Not defined when @ref result_type is `bool` or any non-integral type.
    ///
    /// [1]: @ref Random_source::Random_source(result_type, result_type)
    IF_COMPILER(DECLARE_IF(
            Is_Integral<result_type> &&
            !Is_Same<result_type, bool>))
    explicit Random_source(result_type limit);


    /// Constructs a random source that generates `bool`s, producing `true`
    /// with probability `p_true`.
    ///
    /// Only defined when @ref result_type is `bool`.
    ///
    /// \example
    ///
    /// ```cxx
    /// ge211::Random_source<bool> fair_coin(0.5);
    ///
    /// bool flip_1 = fair_coin.next();
    /// bool flip_2 = fair_coin.next();
    /// bool flip_3 = fair_coin.next();
    ///
    /// if (flip_1 && flip_2 && flip_3) {
    ///     std::cout << "All three flips were heads!\n";
    /// } else if (!(flip_1 || flip_2 || flip_3)) {
    ///     std::cout << "All three flips were tails!\n";
    /// }
    /// ```
    ///
    /// ### Errors
    ///
    ///  - Throws `ge211::Client_logic_error` if `p_true` is less than 0.0 or
    ///    greater than 1.0.
    IF_COMPILER(DECLARE_IF(Is_Same<result_type, bool>))
    explicit Random_source(double p_true);


    /// Constructs a @ref Random_source with no predetermined bounds.
    ///
    /// Use this with
    /// @ref Random_source::operator()(result_type, result_type)
    /// or
    /// @ref Random_source::next_between(result_type, result_type)
    /// to specify the range each time you generate a new number.
    ///
    /// Not defined when @ref result_type is `bool`.
    ///
    /// \example
    ///
    /// In this example, we generate one random uppercase letter,
    /// one random lowercase letter, and one random digit:
    ///
    /// ```
    /// ge211::Random_source<char> source(ge211::unbounded);
    /// char upper = source('A', 'Z');
    /// char lower = source('a', 'z');
    /// char digit = source('0', '9');
    /// ```
    ///
    IF_COMPILER(DECLARE_IF(!Is_Same<result_type, bool>))
    explicit Random_source(unbounded_type);


    /// Returns the next random value from this source.
    ///
    /// \example
    ///
    /// ```cxx
    /// ge211::Random_source<int> one_to_ten(1, 10);
    ///
    /// int choice = one_to_ten.next();
    /// std::cout << "The random number is " << choice << "\n";
    /// ```
    result_type next()
    {
        return engine_->next();
    }

    /// Returns the next random value from this source.
    ///
    /// (This is an alias for @ref Random_source::next().)
    ///
    /// \example
    ///
    /// ```cxx
    /// void try_it(std::size_t n_trials)
    /// {
    ///     ge211::Random_source<bool> fair_coin(0.5);
    ///     std::size_t heads_count = 0;
    ///
    ///     for (std::size_t i = 0; i < n_trials; ++i) {
    ///         if (fair_coin()) {
    ///             ++heads_count;
    ///         }
    ///     }
    ///
    ///     double heads_frequency = heads_count / (double) n_trials;
    ///
    ///     if (heads_frequency < 0.25 || heads_frequency > 0.75) {
    ///         std::cerr << "Doesn't seem so fair!\n";
    ///     }
    /// }
    /// ```
    result_type operator()()
    {
        return next();
    }


    /// Returns a random value between `lo` and `hi`. When using this member
    /// function, it is not necessary to initialize your @ref Random_source
    /// with bounds, so you should usually construct it using
    /// @ref Random_source::Random_source(unbounded_type).
    ///
    /// For an example, see @ref Random_source::Random_source(unbounded_type).
    ///
    /// ### Testing
    ///
    /// If this @ref Random_source is *stubbed* (*e.g.,* via
    /// @ref Random_source::stub_with(std::initializer_list<result_type>)),
    /// the result is computed using the static function
    /// @ref Random_source::bound_between():
    ///
    /// ```
    /// result_type value = next();  // the next stubbed value
    /// return bound_between(value, lo, hi);
    /// ```
    ///
    /// Not defined when @ref result_type is `bool`.
    IF_COMPILER(DECLARE_IF(!Is_Same<result_type, bool>))
    result_type next_between(result_type lo, result_type hi)
    {
        return engine_->next_between(lo, hi);
    }

    /// Returns a random value between `lo` and `hi`.
    ///
    /// (This is an alias for @ref Random_source::next_between().)
    ///
    /// For an example, see @ref Random_source::Random_source(unbounded_type).
    ///
    /// Not defined when @ref result_type is `bool`.
    IF_COMPILER(DECLARE_IF(!Is_Same<result_type, bool>))
    result_type operator()(result_type lo, result_type hi)
    {
        return next_between(lo, hi);
    }

    /// Configures this %Random_source to return a predetermined sequence of
    /// values.
    ///
    /// After passing in a list of values, the Random_source will return those
    /// values in order, and then cycle through them repeatedly if necessary.
    ///
    /// This is intended for testing, in order to make the values chosen by
    /// some component predicable.
    ///
    /// \example
    ///
    /// ```cxx
    /// struct Two_dice
    /// {
    ///     // Source of random die rolls from 1 to 6:
    ///     ge211::Random_source<int> die_source(1, 6);
    ///
    ///     // Rolls both dice and returns their sum.
    ///     int roll();
    /// };
    ///
    /// int Two_dice::roll()
    /// {
    ///     int first = die_source.next();
    ///     int second = die_source.next();
    ///     return first + second;
    /// }
    ///
    /// TEST_CASE("demonstrate random source stubbing"
    /// {
    ///     Model m;
    ///
    ///     // Perform a random roll and check that the result is in the
    ///     // expected range:
    ///     int sum = m.roll();
    ///     CHECK( 2 <= sum && sum <= 12 );
    ///
    ///     // Stub the source to return the given sequence, repeatedly:
    ///     m.die_source.stub_with({1, 2, 3, 4, 5});
    ///
    ///     // Now we can predict the rolls:
    ///     CHECK( m.roll() == 3 );  // rolls 1 and 2
    ///     CHECK( m.roll() == 7 );  // rolls 3 and 4
    ///     CHECK( m.roll() == 6 );  // rolls 5 and 1
    ///     CHECK( m.roll() == 5 );  // rolls 2 and 3
    ///     CHECK( m.roll() == 9 );  // rolls 4 and 5
    ///     CHECK( m.roll() == 3 );  // rolls 1 and 2
    ///     CHECK( m.roll() == 7 );  // rolls 3 and 4
    /// }
    /// ```
    void stub_with(std::initializer_list<result_type> values);


    /// Stubs this @ref Random_source using a @ref std::vector.
    ///
    /// After passing in a vector of values, the %Random_source will
    /// return those values in order, and then cycle through them repeatedly
    /// if necessary. This works the same as
    /// @ref stub_with(std::initializer_list<result_type>),
    /// so you should see that function for an example.
    void stub_with(std::vector<result_type> values);


    /// Stubs this @ref Random_source to always return the given value.
    ///
    /// If you want to stub multiple values in sequence, see
    /// @ref stub_with(std::initializer_list<result_type>)
    /// and
    /// @ref stub_with(std::vector<result_type>).
    void stub_with(result_type value);


    /// Given a randomly-generated @ref result_type `value`, bounds it
    /// between `lo` and `hi` (inclusive) by adjusting `value`s less than
    /// `lo` to `lo` and `value`s greater than `hi` to `hi`.
    IF_COMPILER(DECLARE_IF(!Is_Same<result_type, bool>))
    static result_type bound_between(
            result_type value, result_type lo, result_type hi);

    /// @ref Random_source%s cannot be move-constructed because they
    /// cannot be moved or copied.
    Random_source(Random_source&& other) = delete;

    /// @ref Random_source%s cannot be move-assigned because they cannot
    /// be moved or copied.
    Random_source& operator=(Random_source&& other) = delete;

private:
    using Engine = detail::random::Random_engine<result_type>;
    using Prng = detail::random::Pseudo_random_engine<result_type>;
    using Stub = detail::random::Stub_random_engine<result_type>;

    std::unique_ptr<Engine> engine_;
};


//
// IMPLEMENTATIONS
//

namespace detail {
namespace random {

using std::begin;
using std::end;

template <typename RESULT_TYPE>
template <typename... Args>
Pseudo_random_engine<RESULT_TYPE>::Pseudo_random_engine(Args&&... args)
        : distribution_{std::forward<Args>(args)...},
          generator_{construct_generator()}
{ }

template <typename RESULT_TYPE>
RESULT_TYPE
Pseudo_random_engine<RESULT_TYPE>::next()
{
    return distribution_(generator_);
}

template <typename RESULT_TYPE>
RESULT_TYPE
Pseudo_random_engine<RESULT_TYPE>::next_between(result_type lo, result_type hi)
{
    return Distribution<result_type>{lo, hi}(generator_);
}

template <typename RESULT_TYPE>
Stub_random_engine<RESULT_TYPE>::Stub_random_engine(container_type&& container)
        : container_(std::move(container)),
          next_(begin(container_))
{
    if (next_ == end(container_)) {
        throw ge211::Client_logic_error{
                "Random_source: cannot stub with empty container"};
    }
}

template <typename RESULT_TYPE>
RESULT_TYPE
Stub_random_engine<RESULT_TYPE>::next()
{
    result_type result = *next_++;
    if (next_ == end(container_)) next_ = begin(container_);
    return result;
}

template <typename RESULT_TYPE>
RESULT_TYPE
Stub_random_engine<RESULT_TYPE>::next_between(result_type lo, result_type hi)
{
    return bound_between<result_type>(next(), lo, hi);
}

}  // end namespace random
}  // end namespace detail

template <typename RESULT_TYPE>
IF_COMPILER(DEFINE_IF)
Random_source<RESULT_TYPE>::Random_source(result_type lo, result_type hi)
        : engine_{std::make_unique<Prng>(lo, hi)}
{ }

template <typename RESULT_TYPE>
IF_COMPILER(DEFINE_IF)
Random_source<RESULT_TYPE>::Random_source(result_type limit)
        : engine_{std::make_unique<Prng>(limit)}
{ }

template <typename RESULT_TYPE>
IF_COMPILER(DEFINE_IF)
Random_source<RESULT_TYPE>::Random_source(double p_true)
        : engine_{std::make_unique<Prng>(p_true)}
{ }

template <typename RESULT_TYPE>
IF_COMPILER(DEFINE_IF)
Random_source<RESULT_TYPE>::Random_source(unbounded_type)
        : Random_source{0, std::numeric_limits<result_type>::max()}
{ }

template <typename RESULT_TYPE>
void
Random_source<RESULT_TYPE>::stub_with(std::vector<result_type> values)
{
    engine_ = std::make_unique<Stub>(std::move(values));
}

template <typename RESULT_TYPE>
void
Random_source<RESULT_TYPE>::stub_with(std::initializer_list<result_type> values)
{
    stub_with(std::vector<result_type>(values));
}

template <typename RESULT_TYPE>
void
Random_source<RESULT_TYPE>::stub_with(result_type value)
{
    stub_with({value});
}

template <typename RESULT_TYPE>
IF_COMPILER(DEFINE_IF)
RESULT_TYPE
Random_source<RESULT_TYPE>::bound_between(
        result_type value,
        result_type lo,
        result_type hi)
{
    return detail::random::bound_between<result_type>(value, lo, hi);
}

}  // end namespace ge211
