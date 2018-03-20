CC = g++
LD = g++
CFLAGS = -std=c++14 -ffreestanding -nostdlib -static -fno-stack-protector -m32 -fno-PIC -I. -fno-rtti -fno-exceptions
LDFLAGS = -Wl,-melf_i386
GRUB ?= $(HOME)/Dev/masys/grub/bin
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

boot.img: a.out
	mkdir -p _boot/boot/grub
	cp a.out _boot/
	cp grub.cfg _boot/boot/grub/
	cp hello.txt logo.txt _boot/
	$(MKRESCUE) -o $@ _boot
	rm -rf _boot

a.out: boot.o kernel.o debug.o mem/alloca.o mem/frames.o mem/paging.o mem/vmmap.o mem/malloc.o dev/io.o util.o
	$(LD) -o $@ -T linkscript $(CFLAGS) $(LDFLAGS) $^

%.o: %.cpp
	$(CC) -o $@ -c $(CFLAGS) $<

%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<

test: boot.img
	qemu-system-i386 -serial mon:stdio -cdrom boot.img # monitor: ^A c

clean:
	rm -f boot.img a.out *.o dev/*.o mem/*.o
