#include "ge211_random.h"

#include <chrono>

using namespace std;

namespace ge211 {

static auto construct_engine()
{
    random_device rd;
    auto time = static_cast<random_device::result_type>(
            chrono::high_resolution_clock()
                    .now().time_since_epoch().count());
    mt19937_64 engine(rd() | time);
    return move(engine);
}

Random::Random()
        : generator_{construct_engine()}
{ }

bool Random::random_bool(double ptrue)
{
    return up_to(1.0) < ptrue;
}

}
