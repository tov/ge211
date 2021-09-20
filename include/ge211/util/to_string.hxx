#pragma once

#include <ostream>
#include <sstream>
#include <string>

namespace util {
namespace to_string {
namespace detail {

inline void
concat_to(std::ostream&)
{ }

template <typename FIRST, typename... REST>
void
concat_to(std::ostream& buf, FIRST&& first, REST&& ... rest)
{
    buf << std::forward<FIRST>(first);
    concat_to(buf, std::forward<REST>(rest)...);
}

}  // end namespace detail


/// Converts any printable type to a `std::string`. Multiple arguments
/// are concatenated.
template <typename... PRINTABLE>
std::string
to_string(PRINTABLE&& ... value)
{
    std::ostringstream buf;
    detail::concat_to(buf, std::forward<PRINTABLE>(value)...);
    return buf.str();
}

} // end namespace to_string
} // end namespace util
