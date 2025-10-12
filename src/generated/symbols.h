#pragma once

typedef struct {
    const char* name;
    unsigned long address;
    unsigned long size;
} symbol;

extern symbol symbols[];
extern unsigned long symbol_count;
