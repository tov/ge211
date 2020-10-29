#pragma once

// This lets us disable `noexcept`. One reason to do this is
// to produce simpler API documentation.

#ifdef GE211_DOXYGEN
  #define  NOEXCEPT_(...)
  #define  NOEXCEPT
#else
  #define  NOEXCEPT_(...)  noexcept(__VA_ARGS__)
  #define  NOEXCEPT        noexcept
#endif

