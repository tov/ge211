#include "test_random.hxx"

using namespace ge211;

using std::initializer_list;


namespace {

template <typename T>
struct Test_data
{
    static initializer_list<T> const stub_list;
};

template <>
struct Test_data<bool>
{
    static initializer_list<bool> const stub_list;
};

template <typename T>
initializer_list<T> const Test_data<T>::stub_list{12, 18, 80, 0, 34};

initializer_list<bool> const Test_data<bool>::stub_list{0, 1, 1, 0, 0, 1};

}  // end anonymous namespace


TEST_SUITE_BEGIN("Random_source");

TEST_CASE("Random_source<bool>")
{
    SUBCASE("bounded") {
        check<bool>(0.5).okay();
        check<bool>(1.5).throws_bounds();
        check<bool>(-0.5).throws_bounds();

        check<bool>(0.0).next().equals(false);
        check<bool>(1.0).next().equals(true);
        check<bool>(0.5).next_with_probability(0.5).throws_unsupported();
    }

    SUBCASE("unbounded") {
        check<bool>(unbounded).okay();

        check<bool>(unbounded).next().throws_unsupported();
        check<bool>(unbounded).next_with_probability(0.5).okay();
        check<bool>(unbounded).next_with_probability(0).equals(false);
        check<bool>(unbounded).next_with_probability(1).equals(true);
        check<bool>(unbounded).next_with_probability(1.5).throws_bounds();
        check<bool>(unbounded).next_with_probability(-0.5).throws_bounds();
    }
}

TEST_CASE_TEMPLATE("Random_source", T, int, unsigned, long, float, double)
{
    /// Different lower bound for signed and unsigned types:
    T const m = std::min<T>(-5, 0);

    SUBCASE("bounded") {
        check<T>(5, 9).okay();
        check<T>(5, 5).okay();
        check<T>(m, 5).okay();
        check<T>(m, m).okay();
        check<T>(9, 5).throws_bounds();

        check<T>(5, 5).next().equals(5);
        check<T>(m, m).next().equals(m);
        check<T>(5, 9).next().is_between(5, 9);
        check<T>(m, 5).next().is_between(m, 5);
        check<T>(5, 9).next_between(0, 5).throws_unsupported();
    }

    SUBCASE("unbounded") {
        check<T>(unbounded).okay();

        check<T>(unbounded).next().throws_unsupported();
        check<T>(unbounded).next_between(5, 5).equals(5);
        check<T>(unbounded).next_between(m, m).equals(m);
        check<T>(unbounded).next_between(5, 9).is_between(5, 9);
        check<T>(unbounded).next_between(m, 5).is_between(m, 5);
        check<T>(unbounded).next_between(9, 5).throws_bounds();
    }
}

TEST_CASE_TEMPLATE("limited Random_source", T, int, unsigned, long)
{
    T const m = std::min<T>(-5, 0);

    check<T>(5).okay();
    check<T>(0).throws_bounds();
    check<T>(m).throws_bounds();

    check<T>(5).next().is_between(0, 4);
    check<T>(1).next().equals(0);
    check<T>(5).next_between(0, 5).throws_unsupported();
}

TEST_CASE_TEMPLATE("stubbed Random_source", T, bool, int, unsigned, double)
{
    Random_source<T> src(unbounded);

    auto stub_list = Test_data<T>::stub_list;
    std::vector<T> stub_vec(stub_list);

    SUBCASE("singleton") {
        stub_vec.resize(1);
        src.stub_with(stub_vec[0]);
    }

    SUBCASE("initializer_list") {
        src.stub_with(stub_list);
    }

    SUBCASE("vector") {
        src.stub_with(stub_vec);
    }

    for (int i = 0; i < 30; ++i) {
        CHECK(src.next() == stub_vec[i % stub_vec.size()]);
    }

    // Re-stub:
    src.stub_with(77);
    CHECK(src.next() == T(77));
    CHECK(src.next() == T(77));
    CHECK(src.next() == T(77));
}


TEST_CASE("Random_source type errors")
{
    struct Foo { };

    (void) Random_source<int>(unbounded);
    (void) Random_source<bool>(.25);
    (void) Random_source<double>(5, 10);

    /* Each line should have a type error: */

    // (void) Random_source<Foo>(unbounded);
    // (void) Random_source<double>(5);
    // Random_source<bool>(.25, .75);
    // Random_source<int>(5, 9).next_with_probability(0.5);
    // Random_source<bool>(0.5).next_between(false, true);
    // Random_source<int>(unbounded).next_with_probability(0.5);
    // Random_source<bool>(unbounded).next_between(false, true);
}

TEST_SUITE_END();
