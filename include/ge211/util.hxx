#pragma once

#include "ge211/util/delete_ptr.hxx"
#include "ge211/util/lazy_ptr.hxx"
#include "ge211/util/name_of_type.hxx"
#include "ge211/util/ring_buffer.hxx"
#include "ge211/util/stringable.hxx"
#include "ge211/util/to_string.hxx"

#include <sstream>
#include <string>

/// Generally useful bits of code that are not GE211-specific.
namespace util {

/// Provides container(s), namely, a fixed-sized ring buffer class.
namespace containers { }

/// Facilities for reflecting compile-time information at run time.
namespace reflection { }

/// Provides smart pointers for automatic deletion and lazy initialization.
namespace pointers { }

/// Provides a variable-arity function for assembling strings.
namespace format { }

}  // end namespace util

namespace ge211 {

using util::format::to_string;

/// Type alias to indicate that the given pointer owns
/// its object.
template <typename OBJECT_TYPE>
using Owned = OBJECT_TYPE*;

/// Type alias to indicate that the given pointer borrows
/// its object.
template <typename OBJECT_TYPE>
using Borrowed = OBJECT_TYPE*;

} // end namespace ge211
