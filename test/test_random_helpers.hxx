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
    Checker(Producer<T> producer)
            : producer_(producer)
    { }

    void okay() const
    {
        CHECK_NOTHROW(producer_());
    }

    void equals(T expected) const
    {
        CHECK(producer_() == expected);
    }

    void is_between(T lo, T hi) const
    {
        auto value = producer_();
        CHECK(lo <= value);
        CHECK(value <= hi);
    }

    void throws_bounds() const
    {
        CHECK_THROWS_AS(producer_(), ge211::Random_source_bounds_error);
    }

    void throws_unsupported() const
    {
        CHECK_THROWS_AS(producer_(), ge211::Random_source_unsupported_error);
    }

protected:
    Producer<T> producer_;
};


// A builder is a checker for Random_source with special logic for
// calling Random_source member functions next, next_between, and
// next_with_probability.
template <typename T>
class Builder : public Checker<ge211::Random_source<T>>
{
public:
    using Source = ge211::Random_source<T>;
    using Super = Checker<Source>;
    using Super::producer_;
    using Super::okay;
    using Super::throws_bounds;

    template <typename ... ARGS>
    explicit Builder(ARGS ... args)
            : Super([=] { return Source(args...); })
    { }

    Checker<T> next() const
    {
        return {[=] {
            return producer_().next();
        }};
    }

    Checker<T> next_between(T lo, T hi) const
    {
        return {[=] {
            return producer_().next_between(lo, hi);
        }};
    }

    Checker<T> next_with_probability(double p) const
    {
        return {[=] {
            return producer_().next_with_probability(p);
        }};
    }
};


// Factory function for inferring the instantiation of the Builder
// template.
template <typename T, typename... R>
Builder<T>
check(R ... args)
{
    return Builder<T>(args...);
}

}  // end test_random_helpers
