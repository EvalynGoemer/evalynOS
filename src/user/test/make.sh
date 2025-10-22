gcc -ffreestanding -fPIC -nostdlib -fno-asynchronous-unwind-tables -fno-stack-protector -c test.c -o test.o
ld -nostdlib -T test.ld -o test.elf test.o
objcopy -O binary test.elf test.bin

cp ./test.bin ../../initramfs
cd ../../../
