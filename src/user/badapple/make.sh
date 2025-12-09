clang -O2 -mgeneral-regs-only -ffreestanding -fPIC -nostdlib -fno-asynchronous-unwind-tables -fno-stack-protector -c badapple.c -o badapple.o
ld -nostdlib -T badapple.ld -o badapple.elf badapple.o
objcopy -O binary badapple.elf badapple.bin

cp ./badapple.bin ../../initramfs
cd ../../../
