.PHONY: kernel user clean
pdcplatform_k = masys_kernel
pdcplatform_u = masys_user

incldirs = includes internals opt/nothread
kincldirs = $(incldirs) platform/$(pdcplatform_k)/includes platform/$(pdcplatform_k)/internals
uincldirs = $(incldirs) platform/$(pdcplatform_u)/includes platform/$(pdcplatform_u)/internals
cfiles = $(wildcard functions/*/*.c) $(wildcard opt/nothread/*.c)
kcfiles = $(cfiles) $(wildcard platform/$(pdcplatform_k)/functions/*/*.c)
ucfiles = $(cfiles) $(wildcard platform/$(pdcplatform_u)/functions/*/*.c)

kofiles_ = $(kcfiles:.c=.o)
uofiles_ = $(ucfiles:.c=.o) \
   	   platform/masys_user/functions/masystd/syscall.o \
   	   platform/masys_user/functions/stdlib/malloc.o

kofiles = $(addprefix kernel_build/, $(kofiles_))
uofiles = $(addprefix user_build/, $(uofiles_))

CC = clang
CXX = clang++

kinclflags = $(foreach i, $(kincldirs), -I$i)
uinclflags = $(foreach i, $(uincldirs), -I$i)

kernel: inclflags = $(kinclflags)
kernel: pdclib_kernel.a

user: inclflags = $(uinclflags)
user: pdclib_user.a

cfl = -ffreestanding -nostdlib -static -fno-stack-protector -fno-PIC -m32 \
      -fno-pic -fno-pie -D_PDCLIB_BUILD $(inclflags) -mno-sse -g
CFLAGS += $(cfl) -std=c11
CXXFLAGS += $(cfl) -std=c++11 -fno-rtti -fno-exceptions

pdclib_kernel.a: $(kofiles)
	ar rcs $@ $(kofiles)

pdclib_user.a: $(uofiles)
	ar rcs $@ $(uofiles)

kernel_build/%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<
kernel_build/%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<
kernel_build/%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

user_build/%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<
user_build/%.o: %.S
	$(CC) -o $@ -c $(CFLAGS) $<
user_build/%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $<

clean:
	rm -f $(kofiles) $(uofiles)

bdirs = $(dir $(kofiles) $(uofiles))
$(shell mkdir -p $(bdirs))

