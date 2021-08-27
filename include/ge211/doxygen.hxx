#pragma once

// Hide things from Doxygen or from the compiler, in order to
// produce simpler API documentation.
#ifdef GE211_DOXYGEN
  #define COMPILER_ONLY(...)
  #define DOXYGEN_ONLY(...)     __VA_ARGS__
  #define DECLARE_IF(...)
  #define DEFINE_IF
  #define NOEXCEPT
#else
  #define COMPILER_ONLY(...)    __VA_ARGS__
  #define DOXYGEN_ONLY(...)
  #define DECLARE_IF(...) \
      template < \
          typename DUMMY  = void, \
          typename ENABLE = std::enable_if_t<(__VA_ARGS__), DUMMY> \
      >
  #define DEFINE_IF \
      template <typename, typename>
  #define NOEXCEPT noexcept
#endif
