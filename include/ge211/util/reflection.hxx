#pragma once

#include <climits>

namespace util {
namespace reflection {
namespace detail {

/// Given a built-in type, returns its name as a static C string.
template <typename TYPE>
constexpr char const*
name_of_type_helper(char const* otherwise = "?")
{
    return otherwise;
}

#define Specialize_name_of_type(Type) \
    template <>                       \
    constexpr char const* \
    name_of_type_helper<Type>(char const*) \
    { return #Type; };

Specialize_name_of_type(char)
Specialize_name_of_type(signed char)
Specialize_name_of_type(unsigned char)
Specialize_name_of_type(short)
Specialize_name_of_type(unsigned short)
Specialize_name_of_type(int)
Specialize_name_of_type(unsigned int)
Specialize_name_of_type(long)
Specialize_name_of_type(unsigned long)
Specialize_name_of_type(float)
Specialize_name_of_type(double)
Specialize_name_of_type(long double)

#ifdef LLONG_MAX
Specialize_name_of_type(long long)
#endif

#ifdef ULLONG_MAX
Specialize_name_of_type(unsigned long long)
#endif

#undef Specialize_name_of_type

constexpr char const question_mark_string[] = "?";

}  // end namespace detail

template <
        typename TYPE,
        char const* OTHERWISE = detail::question_mark_string
>
static constexpr char const*
        name_of_type = detail::name_of_type_helper<TYPE>(OTHERWISE);

}  // end namespace reflection
}  // end namespace util
