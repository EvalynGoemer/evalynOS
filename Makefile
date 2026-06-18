.PHONY: default
default:
	@echo "Available Targets:"
	@echo "  - bootstrap              // Compiles required things to build packages"
	@echo "  - initramfs              // Compiles packages for the initramfs if needed and generates"
	@echo "  - run                    // Compiles the kernel and runs in qemu w/ KVM"
	@echo "  - run-tcg                // Compiles the kernel and runs in qemu w/ TCG"
	@echo "  - run-debug              // Compiles the kernel and runs in qemu w/ TCG & Debugger"
	@echo "  - run-tcg-loongarch64    // Compiles the kernel for loongarch64 and runs in qemu w/ TCG"
	@echo "  - run-debug-loongarch64  // Compiles the kernel for loongarch64 and runs in qemu w/ TCG & Debugger"
	@echo "  - run-tcg-riscv64        // Compiles the kernel for riscv64 and runs in qemu w/ TCG"
	@echo "  - run-debug-riscv64      // Compiles the kernel for riscv64 and runs in qemu w/ TCG & Debugger"
	@echo "  - mkiso                  // Makes an ISO that can be ran (Also rebuilds kernel)"

.PHONY: bootstrap
bootstrap:
	./extras/bootstrap.sh

.PHONY: initramfs
initramfs:
	./extras/generate-initramfs.sh

.PHONY: mkiso
mkiso:
	./extras/compile-kernel.sh
	./extras/generate-iso.sh

.PHONY: run
run:
	./extras/compile-kernel.sh
	./extras/generate-iso.sh
	qemu-system-x86_64 \
		-machine q35,accel=kvm,smm=on -s -smp 4 \
		-cpu host,+x2apic,+invtsc,+pdpe1gb \
		-m 512M \
		-drive if=pflash,format=raw,readonly=on,file=./extras/ovmf-code-x86_64.fd \
		-drive if=pflash,format=raw,readonly=on,file=./extras/ovmf-vars-x86_64.fd \
		-cdrom ./evalynOS.iso \
		-boot d -no-reboot -no-shutdown \
		-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
		-serial stdio

.PHONY: run-tcg
run-tcg:
	./extras/compile-kernel.sh
	./extras/generate-iso.sh
	qemu-system-x86_64 \
		-machine q35 -smp 2 \
		-cpu max,+pdpe1gb,la57=on \
		-M accel=tcg,smm=on -d int -no-reboot -no-shutdown -D qemu_log.txt \
		-m 512M \
		-drive if=pflash,format=raw,readonly=on,file=./extras/ovmf-code-x86_64.fd \
		-drive if=pflash,format=raw,readonly=on,file=./extras/ovmf-vars-x86_64.fd \
		-cdrom ./evalynOS.iso \
		-boot d \
		-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
		-chardev stdio,id=debugcon \
		-device isa-debugcon,chardev=debugcon

.PHONY: run-debug
run-debug:
	KASLR="false" ./extras/compile-kernel.sh
	./extras/generate-iso.sh
	qemu-system-x86_64 \
		-machine q35 \
		-s -S \
		-M accel=tcg,smm=on -d int -no-reboot -no-shutdown -D qemu_log.txt \
		-m 512M \
		-drive if=pflash,format=raw,readonly=on,file=./extras/ovmf-code-x86_64.fd \
		-drive if=pflash,format=raw,readonly=on,file=./extras/ovmf-vars-x86_64.fd \
		-cdrom ./evalynOS.iso \
		-boot d \
		-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
		-chardev stdio,id=debugcon \
		-device isa-debugcon,chardev=debugcon

.PHONY: run-tcg-loongarch64
run-tcg-loongarch64:
	ARCH=loongarch64 ./extras/compile-kernel.sh
	./extras/generate-iso.sh
	qemu-system-loongarch64 \
		-M virt -smp 2 \
		-cpu la464 \
		-device ramfb \
		-device qemu-xhci \
		-device usb-kbd \
		-device usb-tablet \
		-M accel=tcg -no-reboot -no-shutdown \
		-m 512M \
		-drive if=pflash,unit=0,format=raw,file=./extras/ovmf-code-loongarch64.fd,readonly=on \
		-cdrom ./evalynOS.iso -serial stdio \
		-boot d \

.PHONY: run-debug-loongarch64
run-debug-loongarch64:
	KASLR="false" ARCH=loongarch64 ./extras/compile-kernel.sh
	./extras/generate-iso.sh
	qemu-system-loongarch64 \
		-M virt \
		-cpu la464 \
		-device ramfb \
		-device qemu-xhci \
		-device usb-kbd \
		-device usb-tablet \
		-s -S \
		-M accel=tcg -d int -no-reboot -no-shutdown -D qemu_log.txt \
		-m 512M \
		-drive if=pflash,unit=0,format=raw,file=./extras/ovmf-code-loongarch64.fd,readonly=on \
		-cdrom ./evalynOS.iso -serial stdio \
		-boot d

.PHONY: run-tcg-riscv64
run-tcg-riscv64:
	ARCH=riscv64 ./extras/compile-kernel.sh
	./extras/generate-iso.sh
	qemu-system-riscv64 \
		-M virt -smp 2 \
		-cpu rv64 \
		-device ramfb \
		-device qemu-xhci \
		-device usb-kbd \
		-device usb-tablet \
		-M accel=tcg -no-reboot -no-shutdown \
		-m 512M \
		-drive if=pflash,unit=0,format=raw,file=./extras/ovmf-code-riscv64.fd,readonly=on \
		-cdrom ./evalynOS.iso -serial stdio \
		-boot d

.PHONY: run-debug-riscv64
run-debug-riscv64:
	KASLR="false" ARCH=riscv64 ./extras/compile-kernel.sh
	./extras/generate-iso.sh
	qemu-system-riscv64 \
		-M virt \
		-cpu rv64 \
		-device ramfb \
		-device qemu-xhci \
		-device usb-kbd \
		-device usb-tablet \
		-s -S \
		-M accel=tcg -d int -no-reboot -no-shutdown -D qemu_log.txt \
		-m 512M \
		-drive if=pflash,unit=0,format=raw,file=./extras/ovmf-code-riscv64.fd,readonly=on \
		-cdrom ./evalynOS.iso -serial stdio \
		-boot d
