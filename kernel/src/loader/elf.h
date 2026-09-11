#pragma once
#include "mem/address_space.h"
#include <stdint.h>

extern bool verify_elf(void* file);
extern uint64_t load_elf(void* file, address_space_t* addrspace);
