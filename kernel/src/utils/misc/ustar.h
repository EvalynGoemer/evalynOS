#pragma once
#include <stddef.h>

extern void* ustar_lookup(void* archive, const char* filename, size_t* sz_out);
