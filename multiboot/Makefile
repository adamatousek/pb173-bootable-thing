CC = clang-3.8
LD = gcc
CFLAGS = -ffreestanding -nostdlib -static -fno-stack-protector -m32 -fno-PIC
LDFLAGS = -Wl,-melf_i386
GRUB = $(HOME)/prog/grub/bin/
MKRESCUE = env PATH=$$PATH:$(GRUB) grub-mkrescue

boot.img: a.out
	mkdir -p _boot/boot/grub
	cp a.out _boot/
	cp grub.cfg _boot/boot/grub/
	cp hello.txt _boot/
	$(MKRESCUE) -o $@ _boot
	rm -rf _boot

a.out: boot.S kernel.c
	$(CC) -o boot.o -c $(CFLAGS) -nopie boot.S
	$(CC) -o kernel.o -c $(CFLAGS) -nopie kernel.c
	$(LD) -o $@ -T linkscript $(CFLAGS) $(LDFLAGS) boot.o kernel.o

test: boot.img
	qemu-system-i386 -cdrom boot.img

clean:
	rm -f boot.img a.out *.o
