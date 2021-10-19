#include "ge211/random.hxx"
#include "ge211/error.hxx"

#include <algorithm>
#include <chrono>
#include <sstream>

using namespace std;

namespace ge211 {
namespace detail {

double
Param_check<bool>::perform(double p, char const *operation)
{
    if (p == 0.0) {
        return -1.0;
    } else if (0.0 < p && p <= 1.0) {
        return p;
    } else {
        Throw_random_source_error::probability(operation, p);
    }
}

namespace {

static Generator::impl_type
make_generator_impl()
{
    using Clock = chrono::high_resolution_clock;
    auto a = random_device{}();
    decltype(a) b = Clock::now().time_since_epoch().count();
    return Generator::impl_type(a ^ b);
}

static string
word_wrap(string const& text, size_t max_width = 72)
{
    using Iter = string::const_iterator;

    size_t width = 0;

    Iter word = text.begin();
    Iter limit = text.end();

    string buf;
    buf.reserve(text.size());
    auto append = [&buf](char c) { buf.push_back(c); };

    while (word < limit) {
        if (*word == ' ') {
            ++word;
            continue;
        }

        Iter space = find(word, limit, ' ');
        size_t wordlen = space - word;
        string next_word(word, space);

        if (width + wordlen + 1 > max_width) {
            append('\n');
            width = 0;
        } else if (width > 0) {
            append(' ');
            ++width;
        }

        std::for_each(word, space, append);
        width += wordlen;
        word = space;
    }

    return buf;
};

} // end anonymous namespace

Generator::Generator()
        : impl(make_generator_impl()) { }

struct Rnd_src_fmt
{
    string const *p_type;
    string const *p_op;

    explicit Rnd_src_fmt(string const& type)
            : p_type(&type),
              p_op(nullptr) { }

    Rnd_src_fmt(string const& type, string const& op)
            : p_type(&type),
              p_op(&op) { }

    Rnd_src_fmt operator()(string const& s) const
    {
        Rnd_src_fmt copy(*this);
        copy.p_op = &s;
        return copy;
    }
};

static ostream&
operator<<(ostream& o, Rnd_src_fmt const& r)
{
    o << "Random_source<" << *r.p_type << ">";

    if (r.p_op) {
        o << "::" << *r.p_op;
    }

    return o;
}

void
Throw_random_source_error::bounds(
        string const& result_type,
        string const& operation,
        string const& lo,
        string const& hi)
{
    throw_concat_<Random_source_bounds_error>(
            Rnd_src_fmt{result_type, operation},
            ": invalid arguments: lower bound (", lo, ") ",
            "must be less than or equal to upper bound (", hi, ").");
}

void
Throw_random_source_error::limit(
        string const& result_type,
        string const& operation,
        string const& limit)
{
    throw_concat_<Random_source_bounds_error>(
            Rnd_src_fmt{result_type, operation},
            ": invalid argument: limit (", limit, ") must be positive.");
}

void
Throw_random_source_error::probability(
        string const& operation,
        double probability)
{
    throw_concat_<Random_source_bounds_error>(
            Rnd_src_fmt{"bool", operation},
            ": invalid argument: probability (", probability,
            ") must be between 0.0 and 1.0, inclusive.");
}

void
Throw_random_source_error::empty_stub(string const& result_type)
{
    throw_concat_<Random_source_empty_stub_error>(
            Rnd_src_fmt{result_type, "stub_with"},
            ": invalid argument: container must be non-empty");
}

void
Throw_random_source_error::unsupported(
        string const& result_type,
        string const& operation,
        bool has_bounds)
{
    bool is_boolean = result_type == "bool";
    char const *param_type = is_boolean ? "probability" : "bounds";
    ostringstream buf;
    Rnd_src_fmt fmt{result_type};

    buf << fmt(operation)
        << ": cannot be used on a Random_source that was constructed "
        << (has_bounds ? "with " : "without ")
        << (is_boolean ? "a " : "")
        << "pre-specified "
        << param_type
        << ". There are two ways to fix this: "
        << "1) Call ";

    if (has_bounds) {
        buf << fmt("next") << "()";
    } else if (is_boolean) {
        buf << fmt("next_with_probability") << "(double)";
    } else {
        buf << fmt("next_between")
            << "(" << result_type << ", " << result_type << ")";
    }

    buf << " to generate a "
        << result_type
        << " value using the "
        << param_type
        << " passed "
        << (has_bounds ? "to the constructor" : "to that function")
        << ", or 2) Construct your "
        << fmt
        << " using Random_source";

    if (has_bounds) {
        buf << "(ge211::unbounded) to avoid specifying ";
    } else if (is_boolean) {
        buf << "(double) to specify ";
    } else {
        buf << "(" << result_type << ", " << result_type << ") to specify ";
    }

    buf << (is_boolean ? "a fixed probability" : "fixed bounds")
        << " at initialization time.";

    throw_concat_<Random_source_unsupported_error>(buf.str());
}

template <typename ERROR_TYPE, typename ... ARGS>
void
Throw_random_source_error::throw_concat_(ARGS const& ... args)
{
    throw ERROR_TYPE(word_wrap(to_string(args...)));
}

} // end namespace detail
} // end namespace ge211
