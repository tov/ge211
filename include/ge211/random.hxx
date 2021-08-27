#pragma once

#include "error.hxx"
#include "forward.hxx"
#include "doxygen.hxx"

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <type_traits>

namespace ge211 {

struct Unbounded_type
{
};

constexpr Unbounded_type unbounded;

namespace detail {

using namespace ge211;

using std::begin;
using std::end;
using std::move;

/// For building error messages.
struct Throw_random_source_error
{
    [[noreturn]] static void
    bounds(
            char const *result_type,
            char const *operation,
            std::string const& lo,
            std::string const& hi);

    [[noreturn]] static void
    empty_stub(char const *result_type);

    [[noreturn]] static void
    limit(
            char const *result_type,
            char const *operation,
            std::string const& limit);

    [[noreturn]] static void
    probability(
            char const *operation,
            double probability);

    [[noreturn]] static void
    unsupported(
            char const *result_type,
            char const *operation,
            bool has_bounds);

private:
    template <typename ERROR_TYPE, typename ... ARGS>
    [[noreturn]] static void
    throw_concat_(ARGS const& ... args);
};


/// Type for the single-parameter Random_source constructor, determined
/// by the result type.
template <typename RESULT_TYPE>
using Param_type = std::conditional_t<
        std::is_same<RESULT_TYPE, bool>{},
        double,
        RESULT_TYPE>;

/// Helper struct for checking parameters. The reason to use a struct
/// is so that we can specialize the `bool` case (below).
template <typename T>
struct Param_check
{
    /// Throws if [`lo`, `hi`] is invalid.
    /// Otherwise, returns `lo`.
    static T perform(T lo, T hi, char const *operation)
    {
        if (lo <= hi) {
            return lo;
        } else {
            Throw_random_source_error::bounds(
                    name_of_type<T>, operation,
                    to_string(lo), to_string(hi));
        }
    }

    /// Throws if `limit` is invalid. Otherwise, returns `limit`.
    static T perform(T limit, char const *operation)
    {
        if (limit > 0) {
            return limit;
        } else {
            Throw_random_source_error::limit(
                    name_of_type<T>, operation,
                    to_string(limit));
        }
    }
};

/// Partial template specialization for `bool`.
template <>
struct Param_check<bool>
{
    /// Throws if `p` is invalid. Otherwise returns `p`, but with
    /// `0.0` changed to `-1.0`.
    static double perform(double p, char const *operation);
};


/// Wraps a std::mt19937 generator to initialize it properly.
struct Generator
{
    using impl_type = std::mt19937_64;

    impl_type impl;

    Generator();
};


/// Wraps a std::uniform_int_distribution if RESULT_TYPE is an integral type;
/// wraps a std::uniform_real_distribution if RESULT_TYPE is a floating-point
/// type; otherwise produces a static error.
template <typename RESULT_TYPE, typename ENABLE = void>
struct Distribution
{
    static constexpr bool parameter_okay =
            !std::is_same<RESULT_TYPE, RESULT_TYPE>{};
    static_assert(parameter_okay,
                  "ge211::Random_source<RESULT_TYPE> requires "
                  "RESULT_TYPE to be a built-in arithmetic type like int "
                  "or double.");
};

/// Integral distribution.
template <typename RESULT_TYPE>
class Distribution<
        RESULT_TYPE,
        std::enable_if_t<std::is_integral<RESULT_TYPE>{}>
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

    Distribution(result_type count)
            : impl_{0, count - 1}
    { }

    result_type
    operator()(Generator& gen)
    {
        return impl_(gen.impl);
    }
};

/// Floating-point distribution.
template <typename TYPE>
class Distribution<
        TYPE,
        std::enable_if_t<std::is_floating_point<TYPE>{}>
>
{
public:
    using result_type = TYPE;

private:
    using impl_type = std::uniform_real_distribution<result_type>;
    impl_type impl_;

public:
    Distribution(result_type lo, result_type hi)
            : impl_{lo, hi}
    { }

    result_type operator()(Generator& gen)
    {
        return impl_(gen.impl);
    }
};


//
// Random engine interfaces
//

template <typename RESULT_TYPE>
struct Random_engine
{
    using result_type = RESULT_TYPE;

    virtual result_type v_next() = 0;
    virtual result_type v_next(result_type, result_type) = 0;
    virtual ~Random_engine() = default;

protected:
    static constexpr char const
            result_type_str_lit_[] = "result_type",
            *parameter_name_ = name_of_type<result_type, result_type_str_lit_>;
};

template <>
struct Random_engine<bool>
{
    using result_type = bool;

    virtual result_type v_next() = 0;
    virtual result_type v_next(double p_true) = 0;
    virtual ~Random_engine() = default;

protected:
    static constexpr char const *parameter_name_ = "bool";
};


//
// Random engine implementation: pseudo-random
//

// Numeric, unbounded
template <typename RESULT_TYPE>
class Unbounded_engine
        : public Random_engine<RESULT_TYPE>
{
protected:
    using Random_engine<RESULT_TYPE>::parameter_name_;

public:
    using result_type = RESULT_TYPE;

    Unbounded_engine() = default;

    result_type v_next() override
    {
        Throw_random_source_error::unsupported(
                parameter_name_, "next", false);
    }

    result_type v_next(result_type lo, result_type hi) override
    {
        return Distribution<result_type>{lo, hi}(generator_);
    }

private:
    Generator generator_;
};

// Boolean, unbounded
template <>
class Unbounded_engine<bool>
        : public Random_engine<bool>
{
public:
    using result_type = bool;

    Unbounded_engine() = default;

    result_type v_next() override
    {
        Throw_random_source_error::unsupported(
                "bool", "next", false);
    }

    result_type v_next(double p_true) override
    {
        return distribution_(generator_) <= p_true;
    }

private:
    Distribution<double> distribution_{0, 1};
    Generator generator_;
};

// Numeric, bounded
template <typename RESULT_TYPE>
class Bounded_engine
        : public Random_engine<RESULT_TYPE>
{
protected:
    using Random_engine<RESULT_TYPE>::parameter_name_;

public:
    using result_type = RESULT_TYPE;

    Bounded_engine(result_type min, result_type max)
            : distribution_(min, max)
    { }

    DECLARE_IF(
            std::is_integral<result_type>{}
    )
    explicit Bounded_engine(result_type limit)
            : Bounded_engine(0, limit - 1)
    { }

    result_type v_next() override
    {
        return distribution_(generator_);
    }

    result_type v_next(result_type, result_type) override
    {
        Throw_random_source_error::unsupported(
                parameter_name_, "next_between", true);
    }

private:
    Distribution<result_type> distribution_;
    Generator generator_;
};

// Boolean, bounded
template <>
class Bounded_engine<bool>
        : public Unbounded_engine<bool>
{
public:
    using result_type = bool;

    explicit Bounded_engine(double p_true)
            : probability_(p_true)
    { }

    result_type v_next() override
    {
        using Super = Unbounded_engine<bool>;
        return Super::v_next(probability_);
    }

    result_type v_next(double) override
    {
        Throw_random_source_error::unsupported(
                "bool",
                "next_with_probability",
                true);
    }

private:
    double probability_;
};


//
// Random engine implementation: stubbed
//

/// Helper class to manage the stub vector.
template <typename RESULT_TYPE>
class Stub_base
{
protected:
    using result_type = RESULT_TYPE;
    using container_type = std::vector<result_type>;
    using iterator_type = typename container_type::const_iterator;

    explicit Stub_base(container_type&& container)
            : container_(move(container)),
              next_(begin(container_))
    {
        if (next_ == end(container_)) {
            throw Client_logic_error{
                    "Random_source: cannot stub with empty container"};
        }
    }

    result_type next_stubbed_()
    {
        result_type result = *next_++;

        if (next_ == end(container_)) {
            next_ = begin(container_);
        }

        return result;
    }

private:
    container_type container_;
    iterator_type next_;
};


template <typename RESULT_TYPE>
class Stub_engine
        : public Random_engine<RESULT_TYPE>,
          private Stub_base<RESULT_TYPE>
{
public:
    using Super = Stub_base<RESULT_TYPE>;
    using typename Super::result_type;
    using typename Super::container_type;

    // PRECONDITION: !container.empty()
    Stub_engine(container_type&& container)
            : Super(move(container))
    { }

    result_type v_next() override
    {
        return Super::next_stubbed_();
    }

    result_type v_next(result_type, result_type) override
    {
        return v_next();
    }
};

template <>
class Stub_engine<bool>
        : public Random_engine<bool>,
          private Stub_base<bool>
{
public:
    using Super = Stub_base<bool>;
    using typename Super::result_type;
    using typename Super::container_type;

    // PRECONDITION: !container.empty()
    Stub_engine(container_type&& container)
            : Super(move(container))
    { }

    result_type v_next() override
    {
        return Super::next_stubbed_();
    }

    result_type v_next(double) override
    {
        return v_next();
    }
};

}  // end namespace detail


/// A generic class for generating [pseudorandom numbers] in uniform
/// distribution over a specified range.
///
/// \param RESULT_TYPE – the type of value produced by this
///     <tt>%Random_source</tt>
///
/// `RESULT_TYPE` may be referred to via the member type @ref result_type.
///
/// For example:
///
///  - A <tt>%Random_source\<float\></tt> is constructed with
///    [Random_source(float lo, float hi)][1]. Calling
///    @ref next() on it produces a `float` from the closed interval
///    [`lo`, `hi`].
///
///  - A <tt>%Random_source\<unsigned\></tt> is constructed with either
///    [Random_source(int lo, int hi)][1] or with
///    [Random_source(int limit)][2]. Calling @ref next() on it
///    produces an `int` from the closed interval [`lo`, `hi`] in
///    the former case, or from the half-closed–half-open interval
///    [`0`, `limit`) in the latter case.
///
///  - A <tt>%Random_source\<bool\></tt> is constructed with
///    @ref Random_source(double p_true). Calling @ref next() on it
///    produces a `bool` value that is `true` with probability `p_true`.
///
///  - An *unbounded* <tt>%Random_source\<int\></tt> is constructed
///    with @ref Random_source(Unbounded_type). Calling
///    [next_between(int lo, int hi)][3]
///    on it produces an `int` value from the closed interval [`lo`, `hi`].
///
///  - An *unbounded* <tt>%Random_source\<bool\></tt> is also constructed
///    with @ref Random_source(Unbounded_type).
///    Calling @ref next_with_probability(double p_true)
///    on it produces a `bool` value that is `true` with probability
///    `p_true`.
///
/// See the documentation for the constructors and member functions for usage
/// examples.
///
/// #### Testing
///
/// You can *stub* your @ref Random_source in order to predetermine the
/// sequence of values that it will return. For details, see the
/// <tt>%Random_source::stub_with()</tt> member function overloads
/// @ref stub_with(std::vector<result_type>),
/// @ref stub_with(std::initializer_list<result_type>), and
/// @ref stub_with(result_type).
///
/// [pseudorandom numbers]:
///     https://en.wikipedia.org/wiki/Pseudorandom_number_generator
///
/// [1]: @ref Random_source::Random_source(result_type, result_type)
///
/// [2]: @ref Random_source::Random_source(result_type)
///
/// [3]: @ref Random_source::next_between(result_type, result_type)
///
template <typename RESULT_TYPE>
class Random_source
{
public:
    /// The type of value generated by this <tt>%Random_source</tt>. This comes
    /// from the first parameter to the <tt>%Random_source</tt> template.
    using result_type = RESULT_TYPE;

private:
    using Engine_ = detail::Random_engine<result_type>;
    using Bounded_ = detail::Bounded_engine<result_type>;
    using Unbounded_ = detail::Unbounded_engine<result_type>;
    using Stub_ = detail::Stub_engine<result_type>;

    template <typename FIRST, typename... REST>
    FIRST param_check_(FIRST first, REST... rest)
    {
        return detail::Param_check<result_type>::perform(first, rest...);
    }

public:
    /// Constructs a <tt>%Random_source\<result_type\></tt> that produces
    /// numeric values between `lo` and `hi`, inclusive.
    ///
    /// \param lo – the smallest value produced by this source
    /// \param hi – the largest value produced by this source
    ///
    /// Only defined when `result_type` is not `bool`. To generate `bool`s,
    /// see @ref Random_source<bool>::Random_source(double) instead.
    ///
    /// #### Errors
    ///
    /// Throws @ref Random_source_bounds_error unless `lo <= hi`.
    ///
    /// \example
    ///
    /// In this example we simulate rolling a fair, six-sided die
    /// repeatedly until it produces 6:
    ///
    /// ```cxx
    /// // Initialize the source to produce `int`s from 1 to 6:
    /// ge211::Random_source<int> six_sided_die(1, 6);
    ///
    /// // Generate a random roll:
    /// int roll_value = six_sided_die.next();
    ///
    /// while ( roll_value != 6 ) {
    ///     std::cout << "You rolled " << roll_value << "; try again.\n";
    ///     roll_value = six_sided_die.next();
    /// }
    ///
    /// std::cout << "Finally rolled a 6!\n";
    /// ```
    DECLARE_IF(
            !std::is_same<result_type, bool>{}
    )
    Random_source(result_type lo, result_type hi)
            : engine_(new Bounded_(param_check_(lo, hi, "Random_source"),
                                   hi))
    { }


    COMPILER_ONLY(
            DECLARE_IF(
                    std::is_integral<result_type>{}
            )
            explicit
            Random_source(detail::Param_type<result_type> param)
            : engine_ (
            new Bounded_(param_check_(param, "Random_source")))
    { }
    )


    /// Constructs a <tt>%Random_source\<result_type\></tt>
    /// (for integral @ref result_type) that produces the given number of
    /// different values.
    ///
    /// \param limit – the number of distinct values produced by this source.
    ///
    /// In particular, the values produced by this
    /// source will come from the closed interval from `0` to
    /// `limit - 1`. This means that
    /// <tt>%Random_source\<result_type\>(limit)</tt> is equivalent to
    /// [Random_source\<result_type\>(0, limit - 1)][1].
    ///
    /// Note that this constructor is not available when @ref result_type
    /// is `bool` or a floating-point type (*e.g.,* `float` or `double`).
    ///
    /// #### Errors
    ///
    /// Throws @ref Random_source_bounds_error unless `limit`
    /// is positive (when @ref result_type is not `bool`).
    ///
    /// \example
    ///
    /// In this example we use a
    /// <tt>%Random_source\<size_t\></tt> to produce random indices into a
    /// vector. In particular, We define define a class template that
    /// provides an operation to random chooses
    /// values from a given list of options. In the constructor, we
    /// initialize our randomness source `source_` with the size of the
    /// vector of options so that it will produce valid indices into that
    /// vector:
    ///
    /// ```cxx
    /// template <typename T>
    /// class Random_among
    /// {
    ///     // Holds the values that we will select from among:
    ///     std::vector<T> options_;
    ///
    ///     // Generates random indices into the `options_` vector:
    ///     ge211::Random_source<std::size_t> source_;
    ///
    /// public:
    ///     // Constructs a `Random_among` that chooses among the given
    ///     // `options`.
    ///     explicit Random_among(std::initialization_list<T> options)
    ///             : options_(options),
    ///               source_(options_.size())
    ///     { }
    ///
    ///     // Returns a random value from among the specified options.
    ///     T const& next()
    ///     {
    ///         return options_[source_.next()];
    ///     }
    /// };
    ///
    /// void
    /// print_random_day(std::ostream& out)
    /// {
    ///     Random_among<char const*> day_of_the_week_source {
    ///         "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    ///     };
    ///
    ///     auto const chosen_day = day_of_the_week_source.next();
    ///
    ///     out << "The day might be " << chosen_day << ".\n";
    /// }
    /// ```
    ///
    /// [1]: @ref Random_source::Random_source(result_type, result_type)
    DOXYGEN_ONLY(
            explicit Random_source(result_type limit);
    )


    /// Constructs a <tt>%Random_source\<bool\></tt> that produces `true`
    /// with the given probability.
    ///
    /// \param p_true – the probability of this source producing `true`
    ///
    /// Note that this constructor is only available when @ref result_type
    /// is `bool`.
    ///
    /// #### Errors
    ///
    /// Throws @ref Random_source_probability_error unless
    /// `limit` is in the closed interval from `0.0` and
    /// `1.0` (when @ref result_type is `bool`).
    ///
    /// \example
    ///
    /// In this example we use a <tt>%Random_source\<bool\></tt> to simulate
    /// a fair coin. We flip the coin `n_trials` times and return the
    /// proportion of heads flips in our sample:
    ///
    /// ```cxx
    /// double try_it(std::size_t n_trials)
    /// {
    ///     ge211::Random_source<bool> fair_coin(0.5);
    ///     std::size_t heads_count = 0;
    ///
    ///     for (std::size_t i = 0; i < n_trials; ++i) {
    ///         if (fair_coin.next()) {
    ///             ++heads_count;
    ///         }
    ///     }
    ///
    ///     return heads_count / double(n_trials);
    /// }
    /// ```
    DOXYGEN_ONLY(
            explicit Random_source(double p_true);
    )


    /// Constructs <tt>%Random_source\<result_type\></tt>
    /// with no predetermined bounds or probability.
    ///
    /// To select this constructor overload, pass the tag value @ref
    /// ge211::unbounded to it, like so:
    ///
    /// ```cxx
    /// ge211::Random_source<long> unbounded_long_src(ge211::unbounded);
    ///
    /// ge211::Random_source<bool> unbounded_bool_src(ge211::unbounded);
    /// ```
    ///
    /// How to use the constructed object depends on its @ref result_type:
    ///
    ///  - If @ref result_type is a numeric type
    ///    (*i.e.,* a non-`bool` [arithmetic type]), use
    ///    @ref Random_source::next_between(result_type, result_type)
    ///    to specify the range each time you generate a new number. For
    ///    examples, see @ref next_between().
    ///
    ///  - If @ref result_type is `bool`, use
    ///    @ref Random_source::next_with_probability(double p_true) to produce
    ///    `true` with probability `p_true`. For an example, see
    ///    @ref next_with_probability().
    ///
    /// [arithmetic type]:
    ///     https://en.cppreference.com/w/c/language/arithmetic_types
    explicit Random_source(Unbounded_type)
            : engine_(new Unbounded_)
    { }


    /// Move-constructs a <tt>%Random_source</tt>.
    ///
    /// (Copy-construction is disallowed.)
    Random_source(Random_source&& other)
            : engine_(std::move(other.engine_))
    { }


    /// Returns the next random value from this <tt>%Random_source</tt>.
    ///
    /// Only defined when @ref Random_source::boundedness is @ref bounded.
    ///
    /// For examples see the constructors:
    ///
    ///   - @ref Random_source::Random_source(result_type lo, result_type hi)
    ///
    ///   - @ref Random_source::Random_source(result_type limit)
    ///
    ///   - @ref Random_source<bool>::Random_source(double p_true)
    ///
    /// #### Errors
    ///
    /// Throws @ref Random_source_unsupported_error if this randomness source
    /// is unbounded. If this happens, you can fix the problem by
    ///
    ///   - using @ref Random_source::next_between(result_type, result_type)
    ///     if @ref result_type is not `bool`,
    ///     or using @ref Random_source::next_with_probability(double)
    ///     if @ref result_type is `bool`; or
    ///
    ///   - providing bounds (or a probability) to the <tt>%Random_source</tt>
    ///     constructor.
    ///
    result_type next()
    {
        return engine_->v_next();
    }


    /// Returns a random value between `lo` and `hi` (inclusive).
    ///
    /// Only defined for
    /// <tt>%Random_source\<result_type, unbounded\></tt>
    /// where @ref result_type is not `bool`.
    ///
    /// #### Errors
    ///
    /// Throws @ref Random_source_bounds_error unless `lo <= hi`.
    ///
    /// \examples
    ///
    /// In this example we generate one random uppercase letter,
    /// one random lowercase letter, and one random digit:
    ///
    /// ```cxx
    /// ge211::Random_source<char, ge211::unbounded> source;
    /// char upper = source.next_between('A', 'Z');
    /// char lower = source.next_between('a', 'z');
    /// char digit = source.next_between('0', '9');
    /// ```
    ///
    /// In this example we implement a [Fisher–Yates shuffle][1],
    /// which requires an unbounded randomness source because each
    /// random index `j` must be drawn from a different range.
    ///
    /// ```cxx
    /// template <typename ELEMENT>
    /// void
    /// shuffle(std::vector<ELEMENT>& vec)
    /// {
    ///     using std::size_t;
    ///     using std::swap;
    ///
    ///     ge211::Random_source<size_t, ge211::unbounded> source;
    ///
    ///     for (size_t i = 0; i < vec.size() - 1; ++i) {
    ///          size_t j = source.next_between(i, vec.size() - 1);
    ///          swap(vec[i], vec[j]);
    ///     }
    /// }
    /// ```
    ///
    /// [1]: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
    DECLARE_IF(
            !std::is_same<result_type, bool>{}
    )
    result_type next_between(result_type lo, result_type hi)
    {
        return engine_->v_next(
                param_check_(lo, hi, "next_between"),
                hi);
    }


    /// Returns `true` with probability `p_true`.
    ///
    /// Only defined for <tt>%Random_source\<bool, unbounded\></tt>.
    ///
    /// #### Errors
    ///
    /// Throws @ref Random_source_probability_error if `p_true` is not in the
    /// closed interval from `0.0` to `1.0`.
    ///
    /// \example
    ///
    /// In this example we generate `bool`s using the probabilities
    /// given by `coins` and return the number of `true`s generated:
    ///
    /// ```cxx
    /// std::size_t
    /// flip_unfair_coins(std::vector<double> const& coins)
    /// {
    ///     ge211::Random_source<bool, ge211::unbounded> source;
    ///     std::size_t true_count = 0;
    ///
    ///     for (double p_true : coins)
    ///         if (source.next_with_probability(p_true)) {
    ///             ++true_count;
    ///         }
    ///     }
    ///
    ///     return true_count;
    /// }
    /// ```
    DECLARE_IF(
            std::is_same<result_type, bool>{}
    )
    bool next_with_probability(double p_true)
    {
        return engine_->v_next(
                param_check_(p_true,
                             "next_with_probability"));
    }


    /// Configures this <tt>%Random_source</tt> to return a predetermined
    /// sequence of values.
    ///
    /// After passing in a list of values, the <tt>%Random_source</tt> will
    /// return those values in order, and then cycle through them repeatedly
    /// if necessary.
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
    void stub_with(std::initializer_list<result_type> values)
    {
        stub_with(std::vector<result_type>(values));
    }

    /// Stubs this <tt>%Random_source</tt> using a @ref std::vector.
    ///
    /// After passing in a vector of values, the <tt>%Random_source</tt> will
    /// return those values in order, and then cycle through them repeatedly
    /// if necessary. This works the same as
    /// @ref stub_with(std::initializer_list<result_type>),
    /// so you should see that function for an example.
    void stub_with(std::vector<result_type> values)
    {
        engine_ = std::make_unique<Stub_>(std::move(values));
    }

    /// Stubs this <tt>%Random_source</tt> to always return the given value.
    ///
    /// If you want to stub multiple values in sequence, see
    /// @ref stub_with(std::initializer_list<result_type>)
    /// and
    /// @ref stub_with(std::vector<result_type>).
    void stub_with(result_type value)
    {
        stub_with(std::vector<result_type>{move(value)});
    }

    /// Move-assigns a <tt>%Random_source</tt>.
    ///
    /// (Copy-assignment is disallowed.)
    Random_source& operator=(Random_source&& other)
    {
        engine_ = std::move(other.engine_);
        return *this;
    }

private:
    std::unique_ptr<Engine_> engine_;
};

}  // end namespace ge211
