#pragma once
#include <stdint.h>
#include <utils/lib.h>

#include STRINGIFY(arch/TARGET_ARCH/intrin/mmio.h)

static inline void mmio_write_offset_64(uint64_t base_addr, uint64_t offset, uint64_t value) { mmio_write_64(base_addr + offset, value); }
static inline uint64_t mmio_read_offset_64(uint64_t base_addr, uint64_t offset) { return mmio_read_64(base_addr + offset); }
static inline void mmio_write_offset_32(uint64_t base_addr, uint64_t offset, uint32_t value) { mmio_write_32(base_addr + offset, value); }
static inline uint32_t mmio_read_offset_32(uint64_t base_addr, uint64_t offset) { return mmio_read_32(base_addr + offset); }
static inline void mmio_write_offset_16(uint64_t base_addr, uint64_t offset, uint16_t value) { mmio_write_16(base_addr + offset, value); }
static inline uint16_t mmio_read_offset_16(uint64_t base_addr, uint64_t offset) { return mmio_read_16(base_addr + offset); }
static inline void mmio_write_offset_8(uint64_t base_addr, uint64_t offset, uint8_t value) { mmio_write_8(base_addr + offset, value); }
static inline uint8_t mmio_read_offset_8(uint64_t base_addr, uint64_t offset) { return mmio_read_8(base_addr + offset); }
