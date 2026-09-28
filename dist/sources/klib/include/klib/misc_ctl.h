#pragma once

#define SYS_MISC_CTL_DBG_PUTC 0xE9

extern void syscall_misc_ctl_putc(char c);
