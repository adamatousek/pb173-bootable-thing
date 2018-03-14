#define ASM_FILE        1
// Stack size shall be at most 4M
#define STACK_SIZE      (36 * 0x1000)
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

        .globl  start, _start, panic
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
        or      $0x3, %eax
        movl    %eax, (%ebx, %ecx, 4)
        dec     %ecx
        jns     loop1

        movl    $VIRT_STACK, %esp       # Map stack to a new virt. addr.
        pushl   $0
        popf

        pushl   %edi /* multiboot information structure */
        pushl   %esi /* multiboot magic value */
        call    main

panic:
pre_halt:
        mov     $2, %ecx
loop_flags_row:
        mov     $39, %ebx
loop_flags_col:
        mov     %ecx, %eax          # row in message
        add     $12, %eax           # row in vga
        mov     $80, %edx
        mul     %edx                # first char of the row on screen
        add     $20, %eax           # move the message to center
        add     %ebx, %eax          # the char on screen
        shl     $1, %eax            # the char in vga
        xor     %dl, %dl
        cmp     $1, %ecx            # middle line
        jne     empty
        movb    halt_message(%ebx), %dl
empty:
        movb    %dl, VIDEO(%eax)
        inc     %eax                # the flag in vga
        movb    $0xE0, VIDEO(%eax)
        dec     %ebx
        jns     loop_flags_col
        dec     %ecx
        jns     loop_flags_row
        hlt
#undef VIDEO

#define HH_PGDIR_IDX (HIGHER_HALF >> 22)
        .section .paging
        .global page_directory
        .align 0x1000
page_directory:
        .long 0x00000083                            # Id-map low 4M
        .fill (HH_PGDIR_IDX - 1), 4, 0x0            # Nothing
        .long 0x00000083                            # Map low 4M to HIGHER_HALF
        .fill (1021 - HH_PGDIR_IDX), 4, 0x0         # Nothing
        .long (stack_pgtbl - HIGHER_HALF + 0x3)     # Page table for the stack
        .long (page_directory - HIGHER_HALF + 0x3)  # Page directory itself

        .align 0x1000
stack_pgtbl:
        .fill 1024, 4, 0x0                          # Nothing yet

halt_message:
        .asciz  "  Nyni muzete pocitac bezpecne vypnout  "
        .comm   stack, STACK_SIZE /* the stack */
