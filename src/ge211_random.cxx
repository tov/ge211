#include "ge211_random.hxx"

#include <chrono>

using namespace std;

namespace ge211 {

namespace detail {

Generator construct_generator()
{
    random_device rd;
    auto time = static_cast<random_device::result_type>(
            chrono::high_resolution_clock()
                    .now().time_since_epoch().count());
    return mt19937_64(rd() ^ time);
}

} // end namespace detail

} // end namespace ge211
