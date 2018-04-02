pdcplatform = masys

incldirs = includes internals opt/nothread \
	   platform/$(pdcplatform)/includes platform/$(pdcplatform)/internals
cfiles = $(wildcard functions/*/*.c) $(wildcard opt/nothread/*.c) \
	 $(wildcard platform/$(pdcplatform)/functions/*/*.c)
ofiles = $(cfiles:.c=.o)

CC = gcc
LD = gcc
inclflags = $(foreach i, $(incldirs), -I$i)
CFLAGS = -ffreestanding -nostdlib -static -fno-stack-protector -fPIC -m32 \
	 -D_PDCLIB_BUILD -std=c11 $(inclflags)

pdclib.a: $(ofiles)
	ar rcs $@ $(ofiles)

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<
