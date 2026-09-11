CC ?= cc
AR ?= ar

CPPFLAGS =
CFLAGS = -ffreestanding -fno-stack-protector -fno-pie -fno-pic -O2 -Iinclude -Wall -Wextra

SRCS = src/klib.c
HDRS = include/klib/syscall.h include/klib/misc_ctl.h
OBJS = $(SRCS:src/%.c=objs/%.o)
DEPS = $(OBJS:.o=.d)
TARGET = libklib.a

$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)

objs/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

install: $(TARGET)
	@mkdir -p $(DESTDIR)/usr/include/klib
	install -Dm644 $(HDRS)   $(DESTDIR)/usr/include/klib/
	install -Dm644 $(TARGET) $(DESTDIR)/usr/lib/$(TARGET)

clean:
	rm -rf objs $(TARGET)

-include $(DEPS)
