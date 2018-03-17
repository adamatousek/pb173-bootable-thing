#define ASM_FILE        1
// Stack size shall be at most 4M
#define STACK_SIZE      (5 * 0x1000)
#define HIGHER_HALF     0xC0000000
#define VIRT_STACK      0xFFC00000
#include "multiboot2.h"

        .section .multiboot
        .align  8 /* required multiboot header alignment */
multiboot_header:
        .long   MULTIBOOT2_HEADER_MAGIC
        .long   MULTIBOOT_ARCHITECTURE_I386
        .long   multiboot_header_end - multiboot_header /* length */
        .long   -( MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 \
                   + ( multiboot_header_end - multiboot_header ) )

        .align 8 /* each tag must be aligned separately */
        .short MULTIBOOT_HEADER_TAG_ADDRESS
        .short 0                        /* flags */
        .long 24                        /* tag size */
        .long (multiboot_header - HIGHER_HALF) /* header address */
        .long (_kernel_start - HIGHER_HALF)    /* load address */
        .long (_kernel_end - HIGHER_HALF)      /* load end address */
        .long (_bss_end - HIGHER_HALF)         /* bss end address */


        .align 8 /* each tag must be aligned separately */
        .short MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
        .short 0                        /* flags */
        .long 12                        /* tag size */
        .long (start - HIGHER_HALF)     /* entry address */

        .align 8 /* each tag must be aligned separately */
        .short MULTIBOOT_HEADER_TAG_END /* tag id */
        .short 0                        /* flags */
        .long 8                         /* tag size */
multiboot_header_end:

        .section .text

        .globl  start, _start, panic, __masys_halt, __masys_hlt
        .type _start, @function

#define VIDEO 0xC00B8000

_start:
start:
multiboot_entry:

        movl    %eax, %esi
        movl    %ebx, %edi

        movl    $(page_directory - HIGHER_HALF), %eax
        movl    %eax, %cr3              # Set phys. addr. of page directory
        mov     %cr4, %eax
        or      $0x00000010, %eax       # Enable PSE
        mov     %eax, %cr4
        mov     %cr0, %eax
        or      $0x80000001, %eax       # Enable paging
        mov     %eax, %cr0

        mov     $higher, %eax
        jmp     *%eax

higher:
        /* map the stack base to 4G minus 4M */
        movl    $((STACK_SIZE >> 12) - 1), %ecx
        movl    $(stack_pgtbl + 0x1000 - (STACK_SIZE >> 10) ), %ebx
loop1:
        movl    %ecx, %eax
        shl     $12, %eax
        add     $(stack - HIGHER_HALF), %eax
        or      $0x103, %eax            # Global, RW, Present
        movl    %eax, (%ebx, %ecx, 4)
        dec     %ecx
        jns     loop1

        movl    $VIRT_STACK, %esp       # Map stack to a new virt. addr.
        pushl   $0
        popf

        pushl   %edi /* multiboot information structure */
        pushl   %esi /* multiboot magic value */
        call    main

__masys_halt:
        lea     halt_message-1, %esi
        movb    $0xE0, %dl
        jmp     pre_halt
panic:
        mov     $(80 * 25 + 1), %ecx
loop_bsod:
        movb    $0x17, (VIDEO - 1)(, %ecx, 2)
        loop    loop_bsod
        lea     panic_message-1, %esi
        movb    $0xC0, %dl
pre_halt:
        std
        mov     $2, %ecx            # 3 rows
loop_flags_row:
        mov     %ecx, %eax          # row in message
        add     $11, %eax           # row in vga
        shl     $4, %eax            # -.
        lea     (%eax,%eax,4), %eax # --`- multiply by 80 (start of row)
        add     $59, %eax           # move the message to center
        mov     $39, %ebx           # 40 columns
        lea     VIDEO(,%eax,2), %edi
loop_flags_col:
        cmp     $1, %ecx            # middle line
        jne     empty
        lodsb
        jmp     flags
empty:
        movb    $'-', %al
flags:
        movb    %dl, %ah            # vga flags
        stosw
        dec     %ebx
        jns     loop_flags_col
        dec     %ecx
        jns     loop_flags_row
__masys_hlt:
        hlt
#undef VIDEO

        .section .rodata
        .ascii  "  Nyni muzete pocitac bezpecne vypnout  "
halt_message:
        .ascii  "   Je nam lito, ale jadro zpanikarilo.  "
panic_message:

#define HH_PGDIR_IDX (HIGHER_HALF >> 22)
        .section .paging
        .global page_directory, kernel_pgtbl
        .align 0x1000
page_directory:
        .long 0x00000083                              # Id-map low 4M
        .fill (HH_PGDIR_IDX - 1), 4, 0x0              # Nothing
        .long 0x00000083                              # Map low 4M to HIGHER_HALF
        .fill (1021 - HH_PGDIR_IDX), 4, 0x0           # Nothing
        .long (stack_pgtbl - HIGHER_HALF + 0x103)     # Page table for the stack
        .long (page_directory - HIGHER_HALF + 0x103)  # Page directory itself

        .align 0x1000
kernel_pgtbl:
        .fill 1024, 4, 0x0                          # Nothing yet

        .align 0x1000
stack_pgtbl:
        .fill 1024, 4, 0x0                          # Nothing yet

        .comm   stack, STACK_SIZE /* the stack */
