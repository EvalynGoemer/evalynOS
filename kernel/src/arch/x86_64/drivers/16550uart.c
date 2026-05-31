#include "arch/x86_64/drivers/portio.h"
#include <arch/x86_64/drivers/16550uart.h>

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// TODO; use uACPI to find valid serial port
// just assume one at the default port for debugging
// also always assume it exists under a hypervisor
// for early logging in VMs even with ACPI
uint16_t serial_port = 0x3F8;
bool serial_enabled = true;
bool serial_works = false;

static inline void serial_set_dlab(uint16_t port, bool setting) {
    uint8_t lcr = inbd(port + SERIAL_LINE_CONF);
    if (setting)
        outbd(port + SERIAL_LINE_CONF, lcr | SERIAL_DLAB_BIT);
    else
        outbd(port + SERIAL_LINE_CONF, lcr & ~SERIAL_DLAB_BIT);
}

static inline void serial_set_divisor(uint16_t port, uint16_t divsor) {
    serial_set_dlab(port, true);
    outbd(port + SERIAL_DLAB_DIV_LO, divsor & 0xff);
    outbd(port + SERIAL_DLAB_DIV_HI, (divsor >> 8) & 0xff);
    serial_set_dlab(port, false);
}

static inline void serial_set_interrupts(uint16_t port, uint8_t setting) {
    serial_set_dlab(port, false);
    outbd(port + SERIAL_INTR_CONF, setting);
}

static inline void serial_set_mcr(uint16_t port, uint8_t setting) {
    outbd(port + SERIAL_MODEM_CONF, setting);
}

static inline void serial_set_lcr(uint16_t port, uint8_t lcr) {
    outbd(port + SERIAL_LINE_CONF, lcr);
}

static inline void serial_set_fifo(uint16_t port, uint8_t fifo) {
    outbd(port + SERIAL_FIFO_CONF, fifo);
}

/// @warning: clobbers serial port config
/// @returns: 2 on success; 1 on partial failure; 0 on total failure
int serial_test(uint16_t port) {
    serial_set_divisor(port, SERIAL_115200_BAUD);
    serial_set_lcr(port, SERIAL_LCR_8BIT | SERIAL_LCR_1STOP | SERIAL_LCR_PARITY_NONE);
    serial_set_fifo(port, SERIAL_FIFO_TX_FLUSH | SERIAL_FIFO_RX_FLUSH);
    serial_set_mcr(port, SERIAL_MCR_TX_ENABLE | SERIAL_MCR_RX_ENABLE | SERIAL_MCR_LOOP_ENABLE);
    serial_set_dlab(port, false);
    for (int i = 0; i < SERIAL_TEST_RETRIES; i++) {
        outbd(port + SERIAL_TX_BUFF, SERIAL_TEST_MAGIC);
        for(int i = 0; i < 256; i++) io_wait();
        if (inbd(port + SERIAL_RX_BUFF) == SERIAL_TEST_MAGIC)
            return 2;
    }

    // fallback test to just make sure it exists at all
    outbd(port + SERIAL_SCRATCH_REG, SERIAL_TEST_MAGIC);
    if (inbd(port + SERIAL_SCRATCH_REG) == SERIAL_TEST_MAGIC)
        return 1;
    return 0;
}

void setup_early_serial() {
    serial_set_interrupts(serial_port, false);
    int status = serial_test(serial_port);
    if(status == 0) {
        serial_works = false;
        LOG_TAGGED("16550 UART", ANSI_RESET, "Failed to init; Do you lack a serial port at I/O port 0x%x?", serial_port)
        LOG_TAGGED_FAIL("16550 UART", ANSI_RESET, "16550 UART INIT")
        return;
    }

    serial_set_divisor(serial_port, SERIAL_115200_BAUD);
    serial_set_lcr(serial_port, SERIAL_LCR_8BIT | SERIAL_LCR_1STOP | SERIAL_LCR_PARITY_NONE);
    serial_set_fifo(serial_port, SERIAL_FIFO_ENABLE | SERIAL_FIFO_THRESH_1b | SERIAL_FIFO_TX_FLUSH | SERIAL_FIFO_RX_FLUSH);
    serial_set_mcr(serial_port, SERIAL_MCR_TX_ENABLE | SERIAL_MCR_RX_ENABLE | SERIAL_MCR_IRQ_ENABLE);
    serial_set_dlab(serial_port, false);
    serial_works = true;

    if (status == 1) {
        LOG_TAGGED("16550 UART", ANSI_RESET, "Serial port at I/O port 0x%x failed part of self test", serial_port)
        LOG_TAGGED_WARN("16550 UART", ANSI_RESET, "16550 UART INIT")
        return;
    }

    LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "16550 UART INIT")
}
