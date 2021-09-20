#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <ge211.hxx>

using namespace ge211::geometry;

TEST_SUITE_BEGIN("geometry");

TEST_CASE_TEMPLATE("the_origin", COORD, int, unsigned long, double)
{
    using Posn = Posn<COORD>;
    CHECK(Posn(the_origin) == Posn(0, 0));
}

TEST_CASE_TEMPLATE("Dims arithmetic",
                   COORD, int, unsigned long, double)
{
    using Dims = Dims<COORD>;

    COORD a, b, c, d;

    SUBCASE("0, 0, 3, 4") {
        a = 0; b = 0; c = 3; d = 4;
    }

    SUBCASE("2, 3, 7, -13") {
        a = 2; b = 3; c = 7; d = -13;
    }

    CHECK(Dims(a, b) + Dims(c, d) == Dims(a + c, b + d));
    CHECK(Dims(a, b) - Dims(c, d) == Dims(a - c, b - d));
    CHECK(-Dims(a, b) == Dims(-a, -b));
    CHECK(d * Dims(a, b) == Dims(d * a, d * b));

    REQUIRE(c > 0);
    CHECK(Dims(c * a, c * b) / c == Dims(a, b));
}

TEST_CASE_TEMPLATE("Mixed Posn/Dims arithmetic",
                   COORD, int, unsigned long, double)
{
    using Posn = Posn<COORD>;
    using Dims = Dims<COORD>;

    Posn a(the_origin), b(3, 4), c(-5, 7);
    Dims ba(b.x, b.y), ca(c.x, c.y), cb(c.x - b.x, c.y - b.y);

    CHECK(b - a == ba);
    CHECK(c - a == ca);
    CHECK(c - b == cb);

    CHECK(a - b == -ba);
    CHECK(a - c == -ca);
    CHECK(b - c == -cb);
}

TEST_CASE_TEMPLATE("Relative position calculations",
                   COORD, int, unsigned long, double)
{
    using Posn = Posn<COORD>;
    using Dims = Dims<COORD>;

    COORD a = 3, b = 4, c = -5, d = 7;

    CHECK(Posn(a, b).down_right_by(Dims(c, d)) == Posn(a + c, b + d));
    CHECK(Posn(a, b).up_right_by(Dims(c, d)) == Posn(a + c, b - d));
    CHECK(Posn(a, b).down_left_by(Dims(c, d)) == Posn(a - c, b + d));
    CHECK(Posn(a, b).up_left_by(Dims(c, d)) == Posn(a - c, b - d));
}

TYPE_TO_STRING(Posn<int>);
TYPE_TO_STRING(Posn<float>);

TEST_CASE_TEMPLATE("Posn<T> to string", T,
                   int, unsigned long, float, double,
                   Posn<int>, Posn<float>)
{
    using util::reflection::name_of_type;

    std::string
            actual = name_of_type<Posn<T>>,
            expected = ge211::to_string("Posn<", name_of_type<T>, ">");

    CHECK( actual == expected);
}

TEST_SUITE_END();

