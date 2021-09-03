#include "test_random.hxx"

using namespace ge211;

TEST_SUITE_BEGIN("Random_source");

TEST_CASE("bool, bounded")
{
    check<bool>(0.5).okay();
    check<bool>(1.5).throws_bounds();
    check<bool>(-0.5).throws_bounds();

    check<bool>(0.0).next().equals(false);
    check<bool>(1.0).next().equals(true);
    check<bool>(0.5).next_with_probability(0.5).throws_unsupported();
}

TEST_CASE("bool, unbounded")
{
    check<bool>(unbounded).okay();

    check<bool>(unbounded).next().throws_unsupported();
    check<bool>(unbounded).next_with_probability(0.5).okay();
    check<bool>(unbounded).next_with_probability(0).equals(false);
    check<bool>(unbounded).next_with_probability(1).equals(true);
    check<bool>(unbounded).next_with_probability(1.5).throws_bounds();
    check<bool>(unbounded).next_with_probability(-0.5).throws_bounds();
}

TEST_CASE("int, bounded")
{
    check<int>(5, 10).okay();
    check<int>(5, 5).okay();
    check<int>(10, 5).throws_bounds();

    check<int>(5, 10).next().is_between(5, 10);
    check<int>(5, 5).next().equals(5);
    check<int>(5, 10).next_between(0, 5).throws_unsupported();
}

TEST_CASE("int, limited")
{
    check<int>(5).okay();
    check<int>(0).throws_bounds();
    check<int>(-1).throws_bounds();

    check<int>(5).next().is_between(0, 4);
    check<int>(5).next_between(0, 5).throws_unsupported();
}

TEST_CASE("int, unbounded")
{
    check<int>(unbounded).okay();

    check<int>(unbounded).next().throws_unsupported();
    check<int>(unbounded).next_between(5, 10).is_between(5, 10);
    check<int>(unbounded).next_between(10, 5).throws_bounds();
}

TEST_CASE("double, bounded")
{
    check<double>(5, 10).okay();
    check<double>(5, 5).okay();
    check<double>(10, 5).throws_bounds();

    check<double>(5, 10).next().is_between(5, 10);
    check<double>(5, 5).next().equals(5);
    check<double>(5, 10).next_between(5, 10).throws_unsupported();
}

TEST_CASE("double, unbounded")
{
    check<double>(unbounded).okay();

    check<double>(unbounded).next().throws_unsupported();
    check<double>(unbounded).next_between(5, 5).equals(5);
    check<double>(unbounded).next_between(5, 10).is_between(5, 10);
    check<double>(unbounded).next_between(10, 5).throws_bounds();
}

#if 0
TEST_CASE("Random_source type errors")
{
    /* Each line here should have a type error: */

    // struct Foo { }; Random_source<Foo> foo(unbounded);
    // Random_source<double>(5.0);
    // Random_source<int>(5, 10).next_with_probability(0.5);
    // Random_source<bool>(0.5).next_between(false, true);
    // Random_source<int>(unbounded).next_with_probability(0.5);
    // Random_source<bool>(unbounded).next_between(false, true);
}
#endif

TEST_SUITE_END();
