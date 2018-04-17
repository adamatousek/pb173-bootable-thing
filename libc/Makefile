.PHONY: kernel user
pdcplatform_k = masys_kernel
pdcplatform_u = masys_user

kincldirs = includes internals opt/nothread \
	    platform/$(pdcplatform_k)/includes platform/$(pdcplatform_k)/internals
uincldirs = includes internals opt/nothread \
	    platform/$(pdcplatform_u)/includes platform/$(pdcplatform_u)/internals
kcfiles = $(wildcard functions/*/*.c) $(wildcard opt/nothread/*.c) \
  	  $(wildcard platform/$(pdcplatform_k)/functions/*/*.c)
ucfiles = $(wildcard functions/*/*.c) $(wildcard opt/nothread/*.c) \
	  $(wildcard platform/$(pdcplatform_u)/functions/*/*.c)
kofiles = $(kcfiles:.c=.o)
uofiles = $(ucfiles:.c=.o) platform/masys_user/functions/stdlib/syscall.o

CC = gcc
LD = gcc

kinclflags = $(foreach i, $(kincldirs), -I$i)
uinclflags = $(foreach i, $(uincldirs), -I$i)

kernel: inclflags = $(kinclflags)
kernel: pdclib_kernel.a
user: inclflags = $(uinclflags)
user: pdclib_user.a

CFLAGS = -ffreestanding -nostdlib -static -fno-stack-protector -fPIC -m32 \
	 -D_PDCLIB_BUILD -std=c11 -no-pie $(inclflags)

pdclib_kernel.a: $(kofiles)
	ar rcs $@ $(kofiles)

pdclib_user.a: $(uofiles)
	ar rcs $@ $(uofiles)

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<
%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<
