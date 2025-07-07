gcc -ffreestanding -fPIC -nostdlib -fno-asynchronous-unwind-tables -fno-stack-protector -c kernel.c -o kernel.o
ld -nostdlib -T kernel.ld -o kernel.elf kernel.o
objcopy -O binary kernel.elf kernel.bin

mkdir ./EFI
mkdir ./EFI/BOOT

gcc -I/usr/include/efi -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c bootloader.c -o bootloader.o
ld -shared -Bsymbolic -L/usr/include/efi -L/usr/lib -L/usr/lib/gnuefi -T/usr/lib/elf_x86_64_efi.lds /usr/lib/crt0-efi-x86_64.o bootloader.o -o bootloader.so -lgnuefi -lefi
objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela  -j .reloc --target efi-app-x86_64 --subsystem=10 bootloader.so bootloader.efi
cp bootloader.efi ./EFI/BOOT/BOOTX64.EFI
qemu-system-x86_64 \
        -machine q35,accel=kvm \
        -cpu host \
        -m 512M \
        -drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd \
        -drive if=pflash,format=raw,file=/usr/share/edk2/x64/OVMF_VARS.4m.fd \
        -drive format=raw,file=fat:rw:. \
        -boot d
