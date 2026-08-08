# EvalynOS

A W.I.P 64 Bit Operating System & Kernel

This kernel uses the [jinx](https://github.com/Mintsuki/Jinx) meta build system to compile. You will need all dependencies for such.

Run `make bootstrap` to build the tools used for compiling the kernel
Run `make bootstrap-bin` to download the tools used for compiling the kernel
Run `make initramfs` to build all packages for the initramfs and generate it  
Run `make run` to build the ISO and run it in QEMU  
Run `make mkiso` to build the ISO

## Supported Platforms

This kernel is currently able to run on the following platforms with varying degrees of support

| Architecture | Support Level | Reason |
|--------------|--------------|--------|
| x86_64 | Full | x86_64 is the main port for the EvalynOS kernel |
| riscv64 | Partial | Dev time is more focused on x86-64. Incomplete Port. |
| loongarch64 | Partial | Dev time is more focused on x86-64. Incomplete Port. Hardware is too expensive |

## Real Hardware
This kernel is able to run on real hardware if it meets the following requirements

| Architecture | Requirements |
|--------------|--------------|
| x86_64 | - x86-64 CPU<br>- 512 MB RAM<br>- PS/2 keyboard and monitor **or** 16550 UART |
| riscv64 | - RV64GC CPU<br>- 512 MB RAM<br>- Able to boot Limine<br>- OpenSBI 0.2+<br>- Monitor **or** serial port |
| loongarch64 | - TBD |

Flash the `evalynOS.iso` file to a usb with a command such as `dd if=./evalynOS.iso of=/dev/{USB_STICK}` and then it can be booted on any system.
