#pragma once

#include "util/delete_ptr.hxx"
#include "util/lazy_ptr.hxx"
#include "util/reflection.hxx"
#include "util/ring_buffer.hxx"

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

}  // end namespace detail


/// Converts any printable type to a `std::string`.
template <typename PRINTABLE>
std::string to_string(const PRINTABLE& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
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
