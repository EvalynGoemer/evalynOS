#pragma once

#define MCE_STATUS_RESTART_IP_VALID (1 << 0)
#define MCE_STATUS_IN_PROGRESS_FLAG (1 << 2)
#define MCE_CAP_BIT_CTL_REG_PRESENT (1 << 8)

extern void setup_mce();
