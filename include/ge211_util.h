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

/// Can type `T` be converted to type `U` without risk of an exception?
template <class T, class U = T>
constexpr bool is_nothrow_convertible()
{
    T t{};
    return noexcept(U(t));
}

/// Can type `T` be compared to itself without risk of an exception?
template<class T>
constexpr bool is_nothrow_comparable()
{
    T t{};
    return noexcept(t == t, t != t);
}

/// Can types `T` and `U` be used for basic arithmetic (addition,
/// subtraction, multiplication) without risk of an exception?
template<class T, class U = T>
constexpr bool has_nothrow_arithmetic()
{
    T t{};
    U u{};
    return noexcept(t + u, t - u, t * u);
}

/// Can types `T` and `U` be used for division without risk of an exception?
template<class T, class U = T>
constexpr bool has_nothrow_division()
{
    T t{};
    U u{};
    return noexcept(t / u);
}

} // end namespace ge211

