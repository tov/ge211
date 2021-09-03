#pragma once

#include "common.hxx"

#include <functional>
#include <utility>

///
/// Helper types
///

namespace {

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

}  // end anonymous namespace
