#pragma once

#include <type_traits>

namespace ge211 {
namespace {

template <typename TYPE_1, typename TYPE_2>
constexpr bool Is_Same = std::is_same<TYPE_1, TYPE_2>::value;

template <typename TYPE>
constexpr bool Is_Arithmetic = std::is_arithmetic<TYPE>::value;

template <typename TYPE>
constexpr bool Is_Integral = std::is_integral<TYPE>::value;

template <typename TYPE>
constexpr bool Is_Floating_Point = std::is_floating_point<TYPE>::value;

#define DECLARE_IF(...) \
            template < \
                typename DUMMY  = void, \
                typename ENABLE = std::enable_if_t<__VA_ARGS__, DUMMY> \
            >

#define DEFINE_IF \
            template <typename DUMMY, typename ENABLE>

}  // end inline detail
}  // end namespace ge211
