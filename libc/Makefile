.PHONY: kernel user clean
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
uofiles = $(ucfiles:.c=.o) \
   	  platform/masys_user/functions/masystd/syscall.o \
   	  platform/masys_user/functions/stdlib/malloc.o

CC = clang
CXX = clang++

kinclflags = $(foreach i, $(kincldirs), -I$i)
uinclflags = $(foreach i, $(uincldirs), -I$i)

kernel: inclflags = $(kinclflags)
kernel: pdclib_kernel.a
user: inclflags = $(uinclflags)
user: pdclib_user.a

CFLAGS += -ffreestanding -nostdlib -static -fno-stack-protector -fno-PIC -m32 \
	  -fno-pic -fno-pie -D_PDCLIB_BUILD -std=c11 $(inclflags) -g -mno-sse
CXXFLAGS += -ffreestanding -nostdlib -static -fno-stack-protector -fno-PIC -m32 \
	    -fno-pic -fno-pie -D_PDCLIB_BUILD -std=c++11 -fno-rtti -fno-exceptions \
	    $(inclflags) -g -mno-sse

pdclib_kernel.a: $(kofiles)
	ar rcs $@ $(kofiles)

pdclib_user.a: $(uofiles)
	ar rcs $@ $(uofiles)

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<
%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<
%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

clean:
	rm -f $(kofiles) $(uofiles)
