CC ?= cc
LD ?= ld.lld

CFLAGS = -ffreestanding -fno-stack-protector -fno-pie -O2 -I/sysroot/usr/include
LDFLAGS ?= -nostdlib -no-pie -L/sysroot/usr/lib -lklib

helloworld.elf: main.o
	$(LD) $(LDFLAGS) -o $@ $<

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f helloworld.elf main.o
