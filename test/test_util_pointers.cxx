#include "doctest.hxx"

#include <ge211/util/delete_ptr.hxx>
#include <ge211/util/lazy_ptr.hxx>

using util::pointers::Delete_ptr;
using util::pointers::Lazy_ptr;

namespace pointer_tester {

struct Delete_ptr_tester
{
    static bool deleted;
    static void go(Delete_ptr_tester *) { deleted = true; }
};

struct Lazy_ptr_tester
{
    static bool forced;
    Lazy_ptr_tester() { forced = true; }
};

bool Lazy_ptr_tester::forced, Delete_ptr_tester::deleted;

}  // end namespace pointer_tester

using namespace pointer_tester;


TEST_SUITE_BEGIN("util::pointers");

TEST_CASE("Delete_ptr")
{
    using T = Delete_ptr_tester;
    using D_f = Delete_ptr<T, T::go>;        // don't delete null
    using D_t = Delete_ptr<T, T::go, true>;  // delete null

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
    Lazy_ptr<T> ptr;

    CHECK_FALSE(T::forced);
    CHECK_FALSE(ptr.is_forced());

    (void) *ptr;

    CHECK(T::forced);
    CHECK(ptr.is_forced());
}

TEST_SUITE_END();
