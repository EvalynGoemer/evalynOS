#pragma once

[[gnu::noreturn]]
void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function);

#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, __func__))

#ifdef NDEBUG
#define dbg_assert(expr) ((void)0)
#else
#define dbg_assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif
