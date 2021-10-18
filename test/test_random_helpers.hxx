#pragma once

#include "doctest.hxx"
#include <ge211/random.hxx>

#include <functional>
#include <utility>

///
/// Helper types
///

namespace test_random_helpers {

/// A function returning type T.
template <typename T>
using Producer = std::function<T()>;

/// Harness for calling a function returning type T and checking
/// the results.
template <typename T>
class Checker
{
public:
    Checker(Producer<T> action)
            : action_(action)
    { }

    void okay() const
    {
        CHECK_NOTHROW(action_());
    }

    void equals(T expected) const
    {
        CHECK(action_() == expected);
    }

    void is_between(T lo, T hi) const
    {
        auto value = action_();
        CHECK(lo <= value);
        CHECK(value <= hi);
    }

    void throws_bounds() const
    {
        CHECK_THROWS_AS(action_(), ge211::Random_source_bounds_error);
    }

    void throws_unsupported() const
    {
        CHECK_THROWS_AS(action_(), ge211::Random_source_unsupported_error);
    }

protected:
    Producer<T> action_;
};

template <typename T>
class Builder : public Checker<ge211::Random_source<T>>
{
public:
    using Source = ge211::Random_source<T>;
    using Super = Checker<Source>;
    using Super::action_;
    using Super::okay;
    using Super::throws_bounds;

    template <typename ... ARGS>
    explicit Builder(ARGS ... args)
            : Super([=] { return Source(args...); })
    { }

    Checker<T> next() const
    {
        return {[=] {
            return action_().next();
        }};
    }

    Checker<T> next_between(T lo, T hi) const
    {
        return {[=] {
            return action_().next_between(lo, hi);
        }};
    }

    Checker<T> next_with_probability(double p) const
    {
        return {[=] {
            return action_().next_with_probability(p);
        }};
    }
};


template <typename T, typename... R>
Builder<T>
check(R ... args)
{
    return Builder<T>(args...);
}

}  // end test_random_helpers
