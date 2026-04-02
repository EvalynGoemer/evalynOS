# EvalynOS

A W.I.P 64 Bit Operating System & Kernel

This kernel uses the [jinx](https://codeberg.org/Mintsuki/jinx) meta build system to compile. You will need all dependancies for such.

Run `make bootstrap` to build the required things for compiling everything else  
Run `make initramfs` to build all packages for the initramfs and generate it  
Run `make run` to build the ISO and run it in QEMU  
Run `make mkiso` to build the ISO

## Real Hardware
This kernel is able to run on real hardware if it meets the following requirements
- x86-64 CPU
- 512MB Memory
- A PS/2 keyboard or a Serial Port
- A Monitor or a Serial Port

Flash the `evalynOS.iso` file to a usb with a command such as `dd if=./evalynOS.iso of=/dev/{USB_STICK}` and then it can be booted om any system.
