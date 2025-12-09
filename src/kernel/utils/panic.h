#pragma once
#include "interupts/interupts.h"

#define PANIC_FLAGS_VECTOR (1 << 0)
#define PANIC_FLAGS_FRAME (1 << 1)
#define PANIC_FLAGS_ERROR (1 << 2)

void panic(char* message, struct interrupt_frame* frame);
