CC = clang++-3.8
LD = g++
CFLAGS = -std=c++14 -ffreestanding -nostdlib -static -fno-stack-protector -m32 -fno-PIC
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

a.out: io.o kernel.o boot.o
	$(LD) -o $@ -T linkscript $(CFLAGS) $(LDFLAGS) $^

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) -nopie $<

%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) -nopie $<

test: boot.img
	qemu-system-i386 -serial stdio -cdrom boot.img

clean:
	rm -f boot.img a.out *.o
