#include "ge211_random.hxx"

#include <chrono>

using namespace std;

namespace ge211 {
namespace detail {
namespace random {

namespace {

double
normalize_probability(double p)
{
    if (p < 0) {
        throw ge211::Client_logic_error{
                "Random_source: probability cannot be < 0"};
    } else if (p > 1) {
        throw ge211::Client_logic_error{
                "Random_source: probability cannot be > 1"};
    }

    return p == 0 ? -1 : p;
}

}  // end inline namespace

Generator
construct_generator()
{
    using Clock = chrono::high_resolution_clock;

    auto a = random_device{}();
    decltype(a) b = Clock::now().time_since_epoch().count();

    return mt19937_64(a ^ b);
}

Pseudo_random_engine<bool>::Pseudo_random_engine(double probability)
        : probability_{normalize_probability(probability)},
          distribution_{0.0, 1.0},
          generator_{construct_generator()}
{ }

bool
Pseudo_random_engine<bool>::next()
{
    return distribution_(generator_) <= probability_;
}

bool
Pseudo_random_engine<bool>::next_between(bool lo, bool hi)
{
    // if lo < hi   return next()
    // if lo > hi   return ! next()
    // if lo == hi  return lo
    return lo ^ (lo != hi && next());
}

} // end namespace detail
} // end namespace random
} // end namespace ge211
