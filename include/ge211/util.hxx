#pragma once

#include "ge211/util/delete_ptr.hxx"
#include "ge211/util/lazy_ptr.hxx"
#include "ge211/util/reflection.hxx"
#include "ge211/util/ring_buffer.hxx"

#include <sstream>
#include <string>

/// Generally useful bits of code that are not GE211-specific.
namespace util {

/// Facilities for reflecting compile-time information at run time.
namespace reflection { }

/// Provides smart pointers for automatic deletion and lazy initialization.
namespace pointers { }

/// Provides container(s), namely, a fixed-sized ring buffer class.
namespace containers { }

}  // end namespace util

namespace ge211 {

namespace detail {

using namespace util::containers;
using namespace util::reflection;
using namespace util::pointers;

inline void
concat_to(std::ostream&)
{ }

template <typename FIRST, typename... REST>
void
concat_to(std::ostream& buf, FIRST&& first, REST&&... rest)
{
    buf << std::forward<FIRST>(first);
    concat_to(buf, std::forward<REST>(rest)...);
}

}  // end namespace detail

/// Converts any printable type to a `std::string`. Multiple arguments
/// are concatenated.
template <typename... PRINTABLE>
std::string
to_string(PRINTABLE&&... value)
{
    std::ostringstream buf;
    detail::concat_to(buf, std::forward<PRINTABLE>(value)...);
    return buf.str();
}


/// Type alias to indicate that the given pointer owns
/// its object.
template <typename OBJECT_TYPE>
using Owned = OBJECT_TYPE*;

/// Type alias to indicate that the given pointer borrows
/// its object.
template <typename OBJECT_TYPE>
using Borrowed = OBJECT_TYPE*;

} // end namespace ge211
