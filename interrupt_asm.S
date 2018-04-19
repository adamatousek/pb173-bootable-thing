#define ASM_FILE        1
#include "config.hpp"

        .section .text
        .global __masys_cr2
        .global __masys_intr_handler, __masys_interrupt_handlers
        .global __masys_setup_idt, __masys_cli, __masys_sti
        .global __masys_userjmp

__masys_cr2:
            mov     %cr2, %eax
            ret

        .align 8
__masys_intr_handler:
        .macro intrhead from, to, target
        .if (\to - \from)
        .align 8
        push    %eax                    # 1-byte opcode
        mov     $(\from), %al           # 2-byte opcode
        jmp     \target                 # 5-byte or 2-byte opcode
        intrhead "(\from + 1)", \to, \target
        .endif
        .endm
            intrhead 0   8   intr_no_errorcode
            intrhead 8   9   intr_no_errorcode
            intrhead 9   10  intr_no_errorcode
            intrhead 10  14  intr_errorcode
            intrhead 14  15  intr_errorcode
            intrhead 15  100 intr_no_errorcode
            intrhead 100 200 intr_no_errorcode
            intrhead 200 256 intr_no_errorcode
intr_no_errorcode:
            pushl   (%esp)              # old eax
            movl    $0xBADC0DE, 4(%esp) # dummy error code
intr_errorcode:
            movzx   %al, %eax
            pushal
            push    40(%esp)            # copy ireturn address to top
            push    40(%esp)            # copy error code to top
            push    %eax
            lea     __masys_interrupt_handlers(, %eax, 4), %eax
            cmp     $0, %eax
            jz      intr_finish
            call    *(%eax)
intr_finish:
            add     $12, %esp
            popal
            pop     %eax
            add     $4, %esp            # error code
            iret


__masys_setup_idt:
            movw        4(%esp), %ax
            movw        %ax, 6(%esp)
            lidt        6(%esp)
            ret

__masys_cli:
            cli
            ret

__masys_sti:
            sti
            ret

__masys_userjmp:
            cli
            mov     $0x23, %ax          # set data segments to user ones
            mov     %ax, %ds
            mov     %ax, %es
            mov     %ax, %fs
            mov     %ax, %gs

            mov     12(%esp), %eax
            mov     (%eax), %edx        # current interrupt stack

            push    %ebp
            mov     %esp, %ebp
            push    %ebx
            push    %esi
            push    %edi
            push    %edx                # save old interrupt stack...
            push    %eax                # ...and where to recover it to
            push    $(MASYS_STACK_MAGIC)
            mov     %esp, (%eax)        # use kernel stack for interrupts

            mov     8(%ebp), %eax
            mov     12(%ebp), %ecx

            sti
            push    $0x23               # fake iret: stack segment
            push    %ecx                # fake iret: user stack bottom
            pushf                       # fake iret: flags
            push    $0x1b               # fake iret: code segment
            push    %eax                # fake iret: destination
            iret

        .section .data
        .global __masys_interrupt_handlers
__masys_interrupt_handlers:
        .align 4
        .fill MASYS_N_INTERRUPTS, 4, 0x0
