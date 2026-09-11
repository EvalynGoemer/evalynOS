#pragma once
#include <sched/scheduler.h>
#include <mem/address_space.h>

extern thread_t* create_kthread(uintptr_t entry, size_t arg1, size_t arg2);
extern thread_t* create_uthread(uintptr_t entry, uintptr_t ustack, address_space_t* addrspace);
extern thread_t* create_uthread_from_elf(const char* initramfs_path);
