# This file was taken and modified from https://codeberg.org/Limine/limine-c-template/raw/commit/c8bc5a2b93397a19272a19a6004b0eeb1e90d982/kernel/GNUmakefile

CFLAGS := -O2 -g -fno-omit-frame-pointer -DMUTE_KERNEL_PANIC

# Nuke built-in rules.
.SUFFIXES:

# This is the name that our final executable will have.
# Change as needed.
override OUTPUT := kernel.elf

# Target architecture to build for. Default to x86_64.
ARCH := x86_64

# Check if the architecture is supported.
ifeq ($(filter $(ARCH), x86_64),)
    $(error Architecture $(ARCH) not supported)
endif

# User controllable toolchain and toolchain prefix.
TOOLCHAIN :=
TOOLCHAIN_PREFIX :=
ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

# User controllable C compiler command.
ifneq ($(TOOLCHAIN_PREFIX),)
    CC := $(TOOLCHAIN_PREFIX)clang
else
    CC := cc
endif

# User controllable linker command.
LD := $(TOOLCHAIN_PREFIX)ld

# Defaults overrides for variables if using "llvm" as toolchain.
ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    LD := ld.lld
endif

# User controllable C preprocessor flags. We set none by default.
CPPFLAGS :=

ifeq ($(ARCH),x86_64)
    # User controllable nasm flags.
    NASMFLAGS :=
endif

# User controllable linker flags. We set none by default.
LDFLAGS :=

# Check if CC is Clang.
override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep -q '^Target: '; echo $$?)

# Internal C flags that should not be changed by the user.
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections \
    -mgeneral-regs-only \
    -mabi=sysv \
    -mno-red-zone \
    -mcmodel=kernel

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
    -I src/kernel \
    -I src/generated \
    -I deps/limine-protocol/include \
    -I deps/flanterm/src/ \
    -I deps/nanoprintf/ \
    -isystem deps/freestnd-c-hdrs/include \
    -isystem src/kernel/libc \
    $(CPPFLAGS) \
    -DLIMINE_API_REVISION=3 \
    -MMD \
    -MP

ifeq ($(ARCH),x86_64)
    # Internal nasm flags that should not be changed by the user.
    override NASMFLAGS := \
        $(patsubst -g,-g -F dwarf,$(NASMFLAGS)) \
        -Wall
endif

# Architecture specific internal flags.
ifeq ($(ARCH),x86_64)
    ifeq ($(CC_IS_CLANG),1)
        override CC += \
            -target x86_64-unknown-none-elf
    endif
    override CFLAGS += \
        -m64 \
        -march=x86-64
    override LDFLAGS += \
        -m elf_x86_64
    override NASMFLAGS := \
        -f elf64 \
        $(NASMFLAGS)
endif

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T src/build-scripts/$(ARCH).lds

# Use "find" to glob all *.c, *.S, and *.asm files in the tree
# (except the src/arch/* directories, as those are gonna be added
# in the next step).
override SRCFILES := $(shell find -L src/kernel -type f -not -path 'src/kernel/arch/*' 2>/dev/null | LC_ALL=C sort)
# Add generated files
override SRCFILES += $(shell find -L src/generated -type f 2>/dev/null | LC_ALL=C sort)
# Add compiled dependencies
override SRCFILES += $(shell find -L deps/cc-runtime/src -type f 2>/dev/null | LC_ALL=C sort)
override SRCFILES += $(shell find -L deps/flanterm/src/  -type f 2>/dev/null | LC_ALL=C sort)
# Add architecture specific files, if they exist.
override SRCFILES += $(shell find -L src/kernel/arch/$(ARCH) -type f 2>/dev/null | LC_ALL=C sort)
# Obtain the object and header dependencies file names.
override CFILES := $(filter %.c,$(SRCFILES))
override ASFILES := $(filter %.S,$(SRCFILES))
ifeq ($(ARCH),x86_64)
override NASMFILES := $(filter %.asm,$(SRCFILES))
endif
override OBJ := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o))
ifeq ($(ARCH),x86_64)
override OBJ += $(addprefix obj-$(ARCH)/,$(NASMFILES:.asm=.asm.o))
endif
override HEADER_DEPS := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

.PHONY: run
run:
	./src/build-scripts/get-deps
	./src/build-scripts/apply-patches.sh
	make clean
	./src/build-scripts/generate-all.sh
	make all -j${nproc}
	./src/build-scripts/undo-patches.sh
	cp ./bin-x86_64/kernel.elf ./kernel.elf
	qemu-system-x86_64 \
		-machine q35,accel=kvm \
		-cpu host \
		-m 512M \
		-drive if=pflash,format=raw,readonly=on,file=./OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,readonly=on,file=./OVMF_VARS.4m.fd \
		-drive format=raw,file=fat:rw:. \
		-boot d \
		-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
		-serial stdio

.PHONY: debug
debug:
	./src/build-scripts/get-deps
	./src/build-scripts/apply-patches.sh
	make clean
	make genclean
	./src/build-scripts/generate-all.sh
	make all -j${nproc}
	cp ./bin-x86_64/kernel.elf ./kernel.elf
	./src/build-scripts/generate-symbols.py
	make clean
	make all -j${nproc}
	./src/build-scripts/undo-patches.sh
	cp ./bin-x86_64/kernel.elf ./kernel.elf
	qemu-system-x86_64 \
		-machine q35,accel=kvm \
		-cpu host \
		-s \
		-S \
		-m 512M \
		-drive if=pflash,format=raw,readonly=on,file=./OVMF_CODE.4m.fd \
		-drive if=pflash,format=raw,readonly=on,file=./OVMF_VARS.4m.fd \
		-drive format=raw,file=fat:rw:. \
		-boot d \
		-audiodev pa,id=speaker -machine pcspk-audiodev=speaker \
		-serial stdio

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf ./bin-$(ARCH)
	rm -rf ./obj-$(ARCH)
	rm -rf ./kernel.elf

# Remove generated files
.PHONY: genclean
genclean:
	rm -rf ./src/generated/*.c
	rm -rf ./src/generated/*.hash
	rm -rf ./initramfs.tar

# Remove downloaded dependencies.
.PHONY: depclean
depclean:
	rm -rf deps/freestnd-c-hdrs deps/cc-runtime deps/limine-protocol deps/flanterm deps/nanoprintf OVMF* EFI

.PHONY: all
all: bin-$(ARCH)/$(OUTPUT)

# Include header dependencies.
-include $(HEADER_DEPS)

# Link rules for the final executable.
bin-$(ARCH)/$(OUTPUT): GNUmakefile src/build-scripts/$(ARCH).lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJ) -o $@

# Compilation rules for *.c files.
obj-$(ARCH)/%.c.o: %.c GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
obj-$(ARCH)/%.S.o: %.S GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ifeq ($(ARCH),x86_64)
# Compilation rules for *.asm (nasm) files.
obj-$(ARCH)/%.asm.o: %.asm GNUmakefile
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) $< -o $@
endif
