#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>

extern void acpi_verify_fadt();

struct [[gnu::packed]] FADT {
    struct SDTHeader header;
    uint32_t firmwareCtrl;
    uint32_t dsdt;

    uint8_t  reserved;

    uint8_t  preferredPowerManagementProfile;
    uint16_t sciInterrupt;
    uint32_t smiCommandPort;
    uint8_t  acpiEnable;
    uint8_t  acpiDisable;
    uint8_t  s4biosRequest;
    uint8_t  pstateControl;
    uint32_t pm1aEventBlock;
    uint32_t pm1bEventBlock;
    uint32_t pm1aControlBlock;
    uint32_t pm1bControlBlock;
    uint32_t pm2ControlBlock;
    uint32_t pmTimerBlock;
    uint32_t gpe0Block;
    uint32_t gpe1Block;
    uint8_t  pm1EventLength;
    uint8_t  pm1ControlLength;
    uint8_t  pm2ControlLength;
    uint8_t  pmTimerLength;
    uint8_t  gpe0Length;
    uint8_t  gpe1Length;
    uint8_t  gpe1Base;
    uint8_t  cStateControl;
    uint16_t worstC2Latency;
    uint16_t worstC3Latency;
    uint16_t flushSize;
    uint16_t flushStride;
    uint8_t  dutyOffset;
    uint8_t  dutyWidth;
    uint8_t  dayAlarm;
    uint8_t  monthAlarm;
    uint8_t  century;

    uint16_t bootArchitectureFlags;

    uint8_t  reserved2;
    uint32_t flags;

    struct ACPIaddr resetReg;

    uint8_t  resetValue;
    uint8_t  reserved3[3];

    uint64_t xFirmwareControl;
    uint64_t xDsdt;

    struct ACPIaddr xPM1aEventBlock;
    struct ACPIaddr xPM1bEventBlock;
    struct ACPIaddr xPM1aControlBlock;
    struct ACPIaddr xPM1bControlBlock;
    struct ACPIaddr xPM2ControlBlock;
    struct ACPIaddr xPMTimerBlock;
    struct ACPIaddr xGPE0Block;
    struct ACPIaddr xGPE1Block;
};

extern struct FADT clean_fadt;

#define FADT_FLAG_HW_REDUCED_ACPI (1 << 20)

[[gnu::always_inline]]
static inline bool acpi_is_hw_reduced() {
    return clean_fadt.flags & FADT_FLAG_HW_REDUCED_ACPI;
}
