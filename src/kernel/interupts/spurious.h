#pragma once

__attribute__((interrupt))
void spurious_isr(__attribute__((unused)) void* frame);
