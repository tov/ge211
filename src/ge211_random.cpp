#include "ge211_random.h"

namespace ge211 {

static auto construct_engine()
{
    std::random_device rd;
    std::mt19937_64 engine(rd());
    return std::move(engine);
}

Random::Random()
        : generator_{construct_engine()}
{ }

bool Random::random_bool(double ptrue)
{
    return up_to(1.0) < ptrue;
}

}
