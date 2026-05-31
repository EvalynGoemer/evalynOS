ARCH   ?= x86_64
KASLR  ?= true

CFLAGS := -O2 -flto -g -fno-omit-frame-pointer
CPPFLAGS :=
LDFLAGS :=

.SUFFIXES:

override OUTPUT := kernel.elf

ifeq ($(ARCH),x86_64)
    TARGET := x86_64-unknown-elf
else ifeq ($(ARCH),loongarch64)
    TARGET := loongarch64-unknown-elf
else
    $(error Architecture $(ARCH) not supported)
endif

CFLAGS += --target=$(TARGET)

CC := clang
LD := ld.lld

override CFLAGS +=        \
    -fdefer-ts            \
    -Wall                 \
    -Wextra               \
    -std=gnu23            \
    -ffreestanding        \
    -ffunction-sections   \
    -fdata-sections       \
    -fstack-protector-all \
    -fsanitize=undefined

ifeq ($(ARCH),x86_64)
    override CFLAGS +=    \
    -m64                  \
    -march=x86-64         \
    -mcmodel=kernel       \
    -mgeneral-regs-only   \
    -mno-red-zone         \
    -mabi=sysv
    override LDFLAGS +=   \
    -m elf_x86_64
    override NASMFLAGS := \
    -w-reloc-abs-qword    \
    -g -F dwarf           \
    -f elf64
endif
ifeq ($(ARCH),loongarch64)
    override CFLAGS +=    \
    -mcmodel=extreme      \
    -march=la464          \
    -mabi=lp64s           \
    -mfpu=none            \
    -msimd=none
    override LDFLAGS +=   \
    -m elf64loongarch     \
    -mllvm='--target-abi=lp64s'
endif

override CPPFLAGS := \
    -I src/                               \
    -I deps/limine-protocol/include       \
    -I deps/flanterm/src/                 \
    -I deps/nanoprintf/                   \
    -I deps/                              \
    -isystem deps/freestnd-c-hdrs/include \
    -isystem src/libc                     \
    $(CPPFLAGS)                           \
    -DLIMINE_API_REVISION=6               \
    -MMD                                  \
    -MP                                   \

override LDFLAGS +=               \
    -nostdlib                     \
    -static                       \
    -z max-page-size=0x1000       \
    --gc-sections                 \
    -T linker_scripts/$(ARCH).lds \
    --build-id=md5                \

ifeq ($(KASLR),true)
override CFLAGS += \
    -fPIC \
    -fpie
override LDFLAGS += \
    --pie
override NASMFLAGS += \
    -dKASLR
else
override CFLAGS += \
    -fno-PIC \
    -fno-pie
override LDFLAGS += \
    --no-pie
endif

override SRCFILES := $(shell find -L src -type f -not -path 'src/arch/*' 2>/dev/null | LC_ALL=C sort)
override SRCFILES += $(shell find -L deps/cc-runtime/src -type f 2>/dev/null | LC_ALL=C sort)
override SRCFILES += $(shell find -L deps/flanterm/src  -type f 2>/dev/null | LC_ALL=C sort)
override SRCFILES += $(shell find -L src/arch/generic -type f 2>/dev/null | LC_ALL=C sort)
override SRCFILES += $(shell find -L src/arch/$(ARCH) -type f 2>/dev/null | LC_ALL=C sort)

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

.PHONY: all
all: bin-$(ARCH)/$(OUTPUT)

.PHONY: clean
clean:
	rm -rf ./bin-$(ARCH)
	rm -rf ./obj-$(ARCH)

-include $(HEADER_DEPS)

bin-$(ARCH)/$(OUTPUT): GNUmakefile linker_scripts/$(ARCH).lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJ) -o $@

obj-$(ARCH)/%.c.o: %.c GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj-$(ARCH)/%.S.o: %.S GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ifeq ($(ARCH),x86_64)
obj-$(ARCH)/%.asm.o: %.asm GNUmakefile
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) $< -o $@
endif
