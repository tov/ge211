#pragma once

#include <ostream>
#include <sstream>
#include <string>

namespace util {
namespace format {
namespace detail {

inline void
concat_to(std::ostream&) { }

template <typename FIRST, typename... REST>
void
concat_to(std::ostream& buf, FIRST const& first, REST const& ... rest)
{
    buf << first;
    concat_to(buf, rest...);
}

}  // end namespace detail


/// Converts any printable type to a `std::string`. Multiple arguments
/// are concatenated.
template <typename... PRINTABLE>
std::string
to_string(PRINTABLE const& ... value)
{
    std::ostringstream buf;
    detail::concat_to(buf, value...);
    return buf.str();
}


} // end namespace format
} // end namespace util
