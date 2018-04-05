.PHONY: libc/pdclib.a test clean
CC = g++
LD = g++
LDFLAGS = -Wl,-melf_i386
GRUB ?= $(HOME)/Dev/masys/grub/bin
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

cincldirs = libc/includes libc/internals libc/opt/nothread\
	    libc/platform/masys/includes libc/platform/masys/internals\
	    libcpp
cinclflags = $(foreach i, $(cincldirs), -I$i)

CFLAGS = -std=c++14 -ffreestanding -nostdlib -static -fno-stack-protector -m32 \
	 -fno-PIC -I. -fno-rtti -fno-exceptions $(cinclflags) -D_PDCLIB_BUILD -g

boot.img: a.out
	mkdir -p _boot/boot/grub
	cp a.out _boot/
	cp grub.cfg _boot/boot/grub/
	cp hello.txt logo.txt _boot/
	$(MKRESCUE) -o $@ _boot
	rm -rf _boot

a.out: boot.o kernel.o debug.o util.o gdt.o interrupt.o interrupt_asm.o \
       mem/alloca.o mem/frames.o mem/paging.o mem/vmmap.o mem/malloc.o \
       dev/io.o libc_glue.o libc/pdclib.a
	$(LD) -o $@ -T linkscript $(CFLAGS) $(LDFLAGS) $^ -static-libgcc -lgcc

%.o: %.cpp
	$(CC) -o $@ -c $(CFLAGS) $<

%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<

#libc/pdclib.a:
#	$(MAKE) -C libc

test: boot.img
	qemu-system-i386 -serial mon:stdio -cdrom boot.img # monitor: ^A c

debug: boot.img
	qemu-system-i386 -S -s -serial mon:stdio -cdrom boot.img # monitor: ^A c

clean:
	rm -f boot.img a.out *.o dev/*.o mem/*.o
