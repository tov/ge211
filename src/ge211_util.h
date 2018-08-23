#pragma once

#include <memory>
#include <string>
#include <sstream>

namespace ge211 {

/// Converts any printable type to a `std::string`.
template<class T>
std::string to_string(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

namespace detail {

template<class T>
using delete_ptr = std::unique_ptr<T, void (*)(T*)>;

template<class T>
void no_op_deleter(T*) {}

} // end namespace detail

}

