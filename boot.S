#define ASM_FILE        1
#define STACK_SIZE      0x4000
#include "multiboot2.h"

        .text

        .globl  start, _start
start:
_start:
        jmp     multiboot_entry

        .align  8 /* required multiboot header alignment */
multiboot_header:
        .long   MULTIBOOT2_HEADER_MAGIC
        .long   MULTIBOOT_ARCHITECTURE_I386
        .long   multiboot_header_end - multiboot_header /* length */
        .long   -( MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 \
                   + ( multiboot_header_end - multiboot_header ) )

        .align 8 /* each tag must be aligned separately */
        .short MULTIBOOT_HEADER_TAG_END /* tag id */
        .short 0                        /* flags */
        .long 8                         /* tag size */
multiboot_header_end:

multiboot_entry:
        movl    $(stack + STACK_SIZE), %esp /* stack pointer */
        pushl   $0
        popf                                /* reset EFLAGS */

        pushl   %ebx /* multiboot information structure */
        pushl   %eax /* multiboot magic value */
        call    main /* let's C */
        jmp     pre_halt

#define VIDEO 0xB8000
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

halt_message:
        .asciz  "  Nyni muzete pocitac bezpecne vypnout  "
        .comm   stack, STACK_SIZE /* the stack */
