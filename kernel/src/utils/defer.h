#pragma once

#if __has_include(<stddefer.h>)
#  include <stddefer.h>

#  if defined(__clang__)
#    if __is_identifier(_Defer)
#      error "you need clang22+ and or -fdefer-ts added to compiler flags"
#    endif
#  else
#    error "stddefer.h" was found but support for your compiler is untested
#  endif

#else
#  error "defer is not supported by the compiler"
#endif
