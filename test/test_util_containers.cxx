#include "doctest.hxx"

#include <ge211/util/ring_buffer.hxx>

using util::containers::Ring_buffer;

TEST_SUITE_BEGIN("util::containers");

TEST_CASE("Ring_buffer (start empty)")
{
    Ring_buffer<int, 4> buf;

    CHECK(buf.empty());
    CHECK_FALSE(buf.full());
    CHECK(buf.size() == 0);

    CHECK(buf.rotate(2) == 0);
    CHECK(buf.rotate(3) == 0);

    CHECK_FALSE(buf.empty());
    CHECK_FALSE(buf.full());
    CHECK(buf.size() == 2);

    CHECK(buf.rotate(4) == 0);
    CHECK(buf.rotate(5) == 0);

    CHECK_FALSE(buf.empty());
    CHECK(buf.full());
    CHECK(buf.size() == 4);

    CHECK(buf.rotate(6) == 2);
    CHECK(buf.rotate(7) == 3);

    CHECK_FALSE(buf.empty());
    CHECK(buf.full());
    CHECK(buf.size() == 4);
}

TEST_CASE("Ring_buffer (start empty)")
{
    Ring_buffer<int, 4> buf;

    CHECK(buf.empty());
    CHECK_FALSE(buf.full());
    CHECK(buf.size() == 0);

    CHECK(buf.rotate(2) == 0);
    CHECK(buf.rotate(3) == 0);

    CHECK_FALSE(buf.empty());
    CHECK_FALSE(buf.full());
    CHECK(buf.size() == 2);

    CHECK(buf.rotate(4) == 0);
    CHECK(buf.rotate(5) == 0);

    CHECK_FALSE(buf.empty());
    CHECK(buf.full());
    CHECK(buf.size() == 4);

    CHECK(buf.rotate(6) == 2);
    CHECK(buf.rotate(7) == 3);

    CHECK_FALSE(buf.empty());
    CHECK(buf.full());
    CHECK(buf.size() == 4);
}

TEST_CASE("Ring_buffer (start filled)")
{
    Ring_buffer<int, 3> buf(100);

    CHECK_FALSE(buf.empty());
    CHECK(buf.full());
    CHECK(buf.size() == 3);

    CHECK(buf.rotate(2) == 100);
    CHECK(buf.rotate(3) == 100);
    CHECK(buf.rotate(4) == 100);
    CHECK(buf.rotate(5) == 2);
    CHECK(buf.rotate(6) == 3);
    CHECK(buf.rotate(7) == 4);
    CHECK(buf.rotate(8) == 5);

    buf.fill(200);

    CHECK_FALSE(buf.empty());
    CHECK(buf.full());
    CHECK(buf.size() == 3);

    CHECK(buf.rotate(10) == 200);
    CHECK(buf.rotate(11) == 200);
    CHECK(buf.rotate(12) == 200);
    CHECK(buf.rotate(13) == 10);
    CHECK(buf.rotate(14) == 11);
}

TEST_SUITE_END();

