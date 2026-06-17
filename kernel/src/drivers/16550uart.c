#include "acpi/tables/sdt.h"
#include <arch/intrin/spin.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <acpi/tables/spcr.h>
#include <arch/intrin/mmio.h>

#ifdef __x86_64__
#include <arch/x86_64/intrin/portio.h>
#endif

#include <drivers/16550uart.h>

serial_ctx_t earlycon_serial = {0};

static inline void serial_write_reg(serial_ctx_t* ctx, uint8_t reg, uint8_t data) {
    #ifdef __x86_64__
    if (ctx->portIO)
        outbd(ctx->addr + reg, data);
    else
        mmio_write_8(ctx->addr + reg, data);
    #else
    assert(!ctx->portIO);
    mmio_write_8(ctx->addr + reg, data);
    #endif
}

static inline uint8_t serial_read_reg(serial_ctx_t* ctx, uint8_t reg) {
    #ifdef __x86_64__
    if (ctx->portIO)
        return inbd(ctx->addr + reg);
    else
        return mmio_read_8(ctx->addr + reg);
    #else
    assert(!ctx->portIO);
    return mmio_read_8(ctx->addr + reg);
    #endif
}

static inline void serial_set_dlab(serial_ctx_t* ctx, bool setting) {
    uint8_t lcr = serial_read_reg(ctx, SERIAL_LINE_CONF);
    if (setting)
        serial_write_reg(ctx, SERIAL_LINE_CONF, lcr | SERIAL_DLAB_BIT);
    else
        serial_write_reg(ctx, SERIAL_LINE_CONF, lcr & ~SERIAL_DLAB_BIT);
}

static inline void serial_set_divisor(serial_ctx_t* ctx, uint16_t divsor) {
    serial_set_dlab(ctx, true);
    serial_write_reg(ctx, SERIAL_DLAB_DIV_LO, divsor & 0xff);
    serial_write_reg(ctx, SERIAL_DLAB_DIV_HI, (divsor >> 8) & 0xff);
    serial_set_dlab(ctx, false);
}

static inline void serial_set_interrupts(serial_ctx_t* ctx, uint8_t setting) {
    serial_set_dlab(ctx, false);
    serial_write_reg(ctx, SERIAL_INTR_CONF, setting);
}

static inline void serial_set_mcr(serial_ctx_t* ctx, uint8_t setting) {
    serial_write_reg(ctx, SERIAL_MODEM_CONF, setting);
}

static inline void serial_set_lcr(serial_ctx_t* ctx, uint8_t lcr) {
    serial_write_reg(ctx, SERIAL_LINE_CONF, lcr);
}

static inline void serial_set_fifo(serial_ctx_t* ctx, uint8_t fifo) {
    serial_write_reg(ctx, SERIAL_FIFO_CONF, fifo);
}

/// @warning: clobbers serial port config
/// @returns: 0 on success; -1 on partial failure; -2 on total failure
int serial_test(serial_ctx_t* ctx) {
    serial_set_divisor(ctx, SERIAL_115200_BAUD);
    serial_set_lcr(ctx, SERIAL_LCR_8BIT | SERIAL_LCR_1STOP | SERIAL_LCR_PARITY_NONE);
    serial_set_fifo(ctx, SERIAL_FIFO_TX_FLUSH | SERIAL_FIFO_RX_FLUSH);
    serial_set_mcr(ctx, SERIAL_MCR_TX_ENABLE | SERIAL_MCR_RX_ENABLE | SERIAL_MCR_LOOP_ENABLE);
    serial_set_dlab(ctx, false);
    for (int i = 0; i < SERIAL_TEST_RETRIES; i++) {
        serial_write_reg(ctx, SERIAL_TX_BUFF, SERIAL_TEST_MAGIC);
        for(int i = 0; i < 4096; i++) spin();
        if (serial_read_reg(ctx, SERIAL_RX_BUFF) == SERIAL_TEST_MAGIC)
            return 0;
    }

    // fallback test to just make sure it exists at all
    serial_write_reg(ctx, SERIAL_SCRATCH_REG, SERIAL_TEST_MAGIC);
    if (serial_read_reg(ctx, SERIAL_SCRATCH_REG) == SERIAL_TEST_MAGIC)
        return -1;
    return -2;
}

int setup_early_serial(uint64_t addr, bool portIO) {
    earlycon_serial.addr = addr;
    earlycon_serial.portIO = portIO;

    serial_set_interrupts(&earlycon_serial, false);
    int status = serial_test(&earlycon_serial);
    if(status == -2)
        return status;

    serial_set_divisor(&earlycon_serial, SERIAL_115200_BAUD);
    serial_set_lcr(&earlycon_serial, SERIAL_LCR_8BIT | SERIAL_LCR_1STOP | SERIAL_LCR_PARITY_NONE);
    serial_set_fifo(&earlycon_serial, SERIAL_FIFO_ENABLE | SERIAL_FIFO_THRESH_1b | SERIAL_FIFO_TX_FLUSH | SERIAL_FIFO_RX_FLUSH);
    serial_set_mcr(&earlycon_serial, SERIAL_MCR_TX_ENABLE | SERIAL_MCR_RX_ENABLE | SERIAL_MCR_IRQ_ENABLE);
    serial_set_dlab(&earlycon_serial, false);
    earlycon_serial.working = true;

    return status;
}

bool detect_early_serial(uint64_t* addr_out, bool* portIO_out) {
    struct SPCR* spcr = acpi_find_sdt(ACPI_SPCR_TABLE_SIGNATURE);
    if (!spcr)
        goto try_dbg2;
    if (!spcr->address.address)
        goto try_dbg2;
    if (spcr->interface_type != SPCR_IFACE_16550)
        goto try_dbg2;
    #ifndef __x86_64__
    if (spcr->address.address_space_id != ACPI_ADDRESS_TYPE_MMIO)
        goto try_dbg2;
    #else
    if (spcr->address.address_space_id != ACPI_ADDRESS_TYPE_MMIO && spcr->address.address_space_id != ACPI_ADDRESS_TYPE_PORT_IO)
        goto try_dbg2;
    #endif

    if (addr_out) *addr_out = spcr->address.address;
    if (portIO_out) *portIO_out = (spcr->address.address_space_id == ACPI_ADDRESS_TYPE_PORT_IO);
    return true;

    // TODO: parse the DBG2 table
    try_dbg2:
    return false;
}

static inline int serial_transmit_empty(serial_ctx_t* ctx) {
    return serial_read_reg(ctx,  SERIAL_LINE_INFO) & SERIAL_TX_EMPTY_BIT;
}

static inline int serial_data_ready(serial_ctx_t* ctx) {
    return serial_read_reg(ctx,  SERIAL_LINE_INFO) & SERIAL_DATA_READY_BIT;
}

void serial_send(serial_ctx_t* ctx, char c) {
    if (!ctx->working)
        return;
    for (int i = 0; i < 100000; i++) {
        if (serial_transmit_empty(ctx))
            break;
        spin();
    }
    serial_write_reg(ctx, SERIAL_TX_BUFF, c);
}

int serial_read(serial_ctx_t* ctx) {
    if (!ctx->working)
        return -1;
    if (!serial_data_ready(ctx))
        return -1;
    return serial_read_reg(ctx,  SERIAL_RX_BUFF);
}
