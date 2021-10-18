#include "doctest.hxx"

#include <ge211/util/to_string.hxx>

using util::format::to_string;
using util::format::Stringable;

TEST_SUITE_BEGIN("util::to_string");

TEST_CASE("to_string")
{
    CHECK(to_string(5) == "5");
    CHECK(to_string(7.5) == "7.5");
    CHECK(to_string(5, ", ", 10, ", ", 15) == "5, 10, 15");
}

TEST_CASE("Stringable")
{
    CHECK(to_string(Stringable(5)) == "5");
    CHECK(to_string(Stringable(7.5)) == "7.5");
    CHECK(to_string(Stringable(5, ", ", 10, ", ", 15)) == "5, 10, 15");
}

TEST_SUITE_END();
