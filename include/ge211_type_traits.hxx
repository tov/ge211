#pragma once

#include <type_traits>

namespace ge211 {
namespace {

template <typename T, typename U>
constexpr bool Is_Same = std::is_same<T, U>::value;

template <typename T>
constexpr bool Is_Arithmetic = std::is_arithmetic<T>::value;

template <typename T>
constexpr bool Is_Integral = std::is_integral<T>::value;

template <typename T>
constexpr bool Is_Floating_Point = std::is_floating_point<T>::value;

template <typename T, typename U>
using Multiply_Result = decltype(std::declval<T>() * std::declval<U>());

#define DECLARE_IF(...) \
            template < \
                typename DUMMY  = void, \
                typename ENABLE = std::enable_if_t<__VA_ARGS__, DUMMY> \
            >

#define DEFINE_IF \
            template <typename DUMMY, typename ENABLE>

}  // end inline detail
}  // end namespace ge211
