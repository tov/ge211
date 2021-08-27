#include "doctest.hxx"
#include <ge211.hxx>
#include <functional>

/// Helper types
namespace {

using namespace ge211;

template <
        typename RESULT_TYPE,
        typename... ARGS
>
struct Check
{
    struct Action;

    Random_source<RESULT_TYPE> src;

    explicit Check(ARGS&& ... args);
    Action next();
    Action next_between(RESULT_TYPE lo, RESULT_TYPE hi);
    Action next_with_probability(double p);
};

template <
        typename RESULT_TYPE,
        typename... ARGS
>
class Check<RESULT_TYPE, ARGS...>::Action
{
    std::function<RESULT_TYPE()> action_;

public:
    Action(std::function<RESULT_TYPE()> action);
    Action const& is_between(RESULT_TYPE lo, RESULT_TYPE hi) const;
    Action const& throws_bounds() const;
    Action const& throws_unsupported() const;
};

///
/// Helper implementations
///

template <typename T, typename... R>
Check<T, R...>::Check(R&& ... args)
        : src{std::forward<R>(args)...}
{ }

template <typename T, typename... R>
auto
Check<T, R...>::next() -> Action
{
    return [=] { return src.next(); };
}

template <typename T, typename... R>
auto
Check<T, R...>::next_between(T lo, T hi) -> Action
{
    return [=] { return src.next_between(lo, hi); };
}

template <typename T, typename... R>
auto
Check<T, R...>::next_with_probability(double p) -> Action
{
    return [=] { return src.next_with_probability(p); };
}

template <typename T, typename... R>
Check<T, R...>::Action::Action(std::function<T()> action)
        : action_(std::move(action))
{ }

template <typename T, typename... R>
auto
Check<T, R...>::Action::is_between(T lo, T hi) const -> Action const&
{
    auto value = action_();
    CHECK(value >= lo);
    CHECK(value <= hi);
    return *this;
}

template <typename T, typename... R>
auto
Check<T, R...>::Action::throws_bounds() const -> Action const&
{
    CHECK_THROWS_AS(action_(), Random_source_bounds_error);
    return *this;
}

template <typename T, typename... R>
auto
Check<T, R...>::Action::throws_unsupported() const -> Action const&
{
    CHECK_THROWS_AS(action_(), Random_source_unsupported_error);
    return *this;
}

}  // end anonymous namespace

