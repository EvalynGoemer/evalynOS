#include "../include/klib/syscall.h"
#include "../include/klib/misc_ctl.h"

void syscall_misc_ctl_putc(char c) {
    syscall2(SYS_MISC_CTL, SYS_MISC_CTL_DBG_PUTC, c);
}
