#pragma once

namespace ge211 {
namespace detail {

template <typename, bool...>
struct Enable_If_All_s;

template <typename Type, bool... Cond>
using Enable_If_All = typename Enable_If_All_s<Type, Cond...>::type;

template <typename Type>
struct Enable_If_All_s<Type>
{
    typedef Type type;
};

template <typename Type, bool... Cond>
struct Enable_If_All_s<Type, true, Cond...>
{
    typedef Enable_If_All<Type, Cond...> type;
};

}
}

// Hide things from Doxygen or from the compiler, in order to
// produce simpler API documentation.
#ifdef GE211_DOXYGEN
  #define DECLARE_IF(...)
  #define DEFINE_IF

  #define NOEXCEPT0
  #define NOEXCEPT(...)
  #define NOEXCEPT_AS(...)
#else
  #define DECLARE_IF(...) \
      template < \
          typename D = void, \
          typename E = typename detail::Enable_If_All<D, __VA_ARGS__> \
      >

  #define DEFINE_IF \
      template <typename, typename>

  #define NOEXCEPT(...)     noexcept(__VA_ARGS__)
  #define NOEXCEPT_         noexcept
  #define NOEXCEPT_AS(...)  noexcept(noexcept(__VA_ARGS__))
#endif
