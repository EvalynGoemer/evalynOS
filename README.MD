# EvalynOS
A W.I.P 64 Bit OS Kernel

## Compiling & Running

You will need to install the following arch packages to compile (Non inclusive, More testing needs to be done)
- base-devel
- python3
- qemu-desktop
- edk2-ovmf
- limine

You will need to run the `get-deps` script before starting

Compile with `make`

Run and compile with `make run`

Run and compile and generate symbols (Compiles twice) with `make debug`

## Compile Flags
- `-DINCLUDE_DEMOS`: Compiles the demos to be bundled with the kernel (Large, Will be moved to userspace later)
- `-DSKIP_TO_DEMOS`: Have kernel automaticaly launch demos if keyboard drivers are not working on your hardware (Will be implemented into userspace with an init system later)
- `-DMUTE_KERNEL_PANIC`: Disables the 1khz tone played on a kernel panic for when doing active work on the kernel
Add these flags to the `GNUmakefile` `CFLAGS` section

## Real Hardware
This kernel is designed to be able to run on *REAL* hardware if it meets the following requirements
- x86-64 CPU
- 512MB Memory
- A 1280x720 or higher monitor (1920x1080 recommended)
- A PS/2 keyboard
- A PC speaker

Copy the `EFI` folder and `limine.conf` and `kernel.elf` to the root of a GPT formatted USB stick with the boot and esp flags set and then boot to it

## Kernel Mode Shell Commands
- `TEST`: Prints a test message
- `CLEAR`: Clears the terminal (ALIAS: `CLS`)
- `CLEARFB`: Clears the framebufer (ALIAS: `CLSFB`)
- `MMAP`: Prints the memory map provided by the bootloader
- `PANIC`: Causes a kernel panic
- `FAULT`: Causes a Page Fault to panic with extended info

## Kernel Mode Shell Demos
- `BADAPPLE`: Plays the Bad Apple music video on your screen with audio via PC speaker
