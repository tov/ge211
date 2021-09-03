#pragma once

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <ge211.hxx>

namespace {

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

}  // end anonymous namespace
