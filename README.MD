# EvalynOS
This is a very W.I.P project

There will be ~~no~~ *some* help given in its current state

This repo is just to track history incase of oopsies

## Some Help

You will need to install the following arch packages to compile (Non inclusive, More testing needs to be done)
- base-devel
- qemu-desktop
- edk2-ovmf
- limine

You will need to run the `get-deps` script before starting

Compile with `make` and run with `make run`

`make run` will always run a `make clean` and `make run` to have a fresh compile

Compile with the included make file and if you wish to build with the demos and or have the demos automaticaly start if the keyboard drivers fail on your hardware edit the makefile (GNUmakefile) at CFLAGS and add -DINCLUDE_DEMOS to have the demos bundled and launchable from the shell and add -DSKIP_TO_DEMOS ontop to have them automaticaly play

## Real Hardware

This kernel is designed to be able to run on *REAL* hardware if it meets the following requirements
- x86-64 CPU
- 512GB Memory
- A 1920x1080 Monitor (If needed source files can be edited for slightly smaller resolutions to better fir but YMMV)
- A PS/2 keyboard
- A PC speaker

Copy the `EFI` folder and `limine.conf` and `kernel.elf` to the root of a GPT formatted USB stick with the boot and esp flags set and then boot to it

## Kernel Mode Shell Commands
- TEST: Prints a test message

## Kernel Mode Shell Demos
- BADAPPLE: Plays the Bad Apple music video on your screen with audio via PC speaker
