#pragma once

#define ISA_BASELINE_RV64GC "rv64imafdc"
#define ISA_EXT_ZBA         "zba"
#define ISA_EXT_ZBB         "zbb"

extern bool isa_has_baseline(const char* isa, const char* required);
extern bool isa_has_extension(const char *isa, const char *ext);
