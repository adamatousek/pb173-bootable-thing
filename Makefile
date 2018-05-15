.PHONY: all kernel user libc/pdclib_kernel.a libc/pdclib_user.a test clean

all: boot.img

kernel: pdcplatform = masys_kernel
kernel: cust_cflags = -I.
kernel: a.out

user: pdcplatform = masys_user

CC = g++
LD = g++
LDFLAGS += -Wl,-melf_i386 -no-pie
GRUB ?= $(HOME)/Dev/masys/grub/bin
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

cincldirs = libc/includes libc/internals libc/opt/nothread\
	    libc/platform/$(pdcplatform)/includes\
	    libc/platform/$(pdcplatform)/internals\
	    libcpp compiler-rt
cinclflags = $(foreach i, $(cincldirs), -I$i)

CFLAGS += -std=c++14 -ffreestanding -nostdlib -static -fno-stack-protector -m32 \
 	  -fno-PIC -fno-rtti -fno-exceptions $(cinclflags) -D_PDCLIB_BUILD \
	  -g $(cust_cflags)

kofiles = boot.o kernel.o debug.o util.o gdt.o interrupt.o interrupt_asm.o \
          syscall/syscall_asm.o syscall/syscall.o syscall/cease.o \
	  syscall/obtain.o syscall/inscribe.o \
       	  userspace.o \
       	  mem/alloca.o mem/frames.o mem/paging.o mem/vmmap.o mem/malloc.o \
       	  dev/io.o libc_glue.o

comprt = compiler-rt/udivdi3.o compiler-rt/umoddi3.o

boot.img: kernel user
	mkdir -p _boot/boot/grub
	cp a.out _boot/
	cp grub.cfg _boot/boot/grub/
	cp hello.txt logo.txt _boot/
	cp hello.text.bin hello.data.bin _boot/
	$(MKRESCUE) -o $@ _boot
	rm -rf _boot

a.out: $(kofiles) $(comprt) libc/pdclib_kernel.a
	$(LD) -o $@ -T linkscript $(CFLAGS) $(LDFLAGS) $^

%.o: %.cpp
	$(CC) -o $@ -c $(CFLAGS) $<

%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<

libc/pdclib_kernel.a:
	$(MAKE) -C libc kernel

test: boot.img
	qemu-system-i386 -serial mon:stdio -cdrom boot.img # monitor: ^A c

debug: boot.img
	qemu-system-i386 -S -s -serial mon:stdio -cdrom boot.img # monitor: ^A c

clean:
	rm -f boot.img a.out *.o dev/*.o mem/*.o syscall/*.o compiler-rt/*.o
	$(MAKE) -C libc clean

#### USERLAND ####

user: hello.text.bin hello.data.bin

hello.elf: USER/hello.o $(comprt) libc/pdclib_user.a
	#$(LD) -o $@ $(CFLAGS) $(LDFLAGS) $^ -static-libgcc -lgcc
	$(LD) -o $@ -T USER/linkscript $(CFLAGS) $(LDFLAGS) $^

USER/hello.o: USER/hello.c
	$(CC) -o $@ -c $(CFLAGS) $<

hello.text.bin: hello.elf
	objcopy -I elf32-i386 -O binary -j .text -S $< $@

hello.data.bin: hello.elf
	objcopy -I elf32-i386 -O binary -j .data -S $< $@

libc/pdclib_user.a:
	$(MAKE) -C libc user

