#include "test_util.hxx"

#include <climits>
#include <string>

using namespace util;
using namespace util_testers;

TEST_SUITE_BEGIN("util");


TEST_CASE("Delete_ptr")
{
    using T = Delete_ptr_tester;
    using D_f = pointers::Delete_ptr<T, T::go>;
    using D_t = pointers::Delete_ptr<T, T::go, true>;

    T t;

    T::deleted = false;
    bool expected;

    SUBCASE("non-null") {
        D_f ptr(&t);
        CHECK_FALSE(T::deleted);
        expected = true;
    }

    SUBCASE("null") {
        D_f ptr(nullptr);
        CHECK_FALSE(T::deleted);
        expected = false;
    }

    SUBCASE("non-null, delete null") {
        D_t ptr(&t);
        CHECK_FALSE(T::deleted);
        expected = true;
    }

    SUBCASE("null, delete null") {
        D_t ptr(nullptr);
        CHECK_FALSE(T::deleted);
        expected = true;
    }

    CHECK(T::deleted == expected);
}


TEST_CASE("Lazy_ptr")
{
    using T = Lazy_ptr_tester;

    T::forced = false;
    pointers::Lazy_ptr<T> ptr;

    CHECK_FALSE(T::forced);
    CHECK_FALSE(ptr.is_forced());

    (void) *ptr;

    CHECK(T::forced);
    CHECK(ptr.is_forced());
}


TEST_CASE("name_of_type")
{
    using T = Name_of_type_tester;

#define DO_CHECK(U)     CHECK(T::go<U>() == #U)
    DO_CHECK(bool);
    DO_CHECK(char);
    DO_CHECK(unsigned char);
    DO_CHECK(signed char);
    DO_CHECK(short);
    DO_CHECK(unsigned short);
    DO_CHECK(int);
    DO_CHECK(unsigned int);
    DO_CHECK(long);
    DO_CHECK(unsigned long);
    DO_CHECK(float);
    DO_CHECK(double);
    DO_CHECK(long double);
#ifdef LLONG_MAX
    DO_CHECK(long long);
#endif
#ifdef ULLONG_MAX
    DO_CHECK(unsigned long long);
#endif
#undef DO_CHECK

    CHECK(T::go<std::string>() == "?");
    CHECK(T::go<std::string, T::unknown>() == T::unknown);
    CHECK(T::go<int, T::unknown>() == "int");
}


TEST_CASE("Ring_buffer (start empty)")
{
    containers::Ring_buffer<int, 4> buf;

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
    containers::Ring_buffer<int, 4> buf;

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
    containers::Ring_buffer<int, 3> buf(100);

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
