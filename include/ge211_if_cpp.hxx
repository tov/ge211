#pragma once

// Hide things from Doxygen or from the compiler.

#ifdef GE211_DOXYGEN
  #define IF_COMPILER(...)
  #define IF_DOXYGEN(...) __VA_ARGS__
#else
  #define IF_COMPILER(...) __VA_ARGS__
  #define IF_DOXYGEN(...)
#endif
