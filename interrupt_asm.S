#define ASM_FILE        1

        .section .text
        .global intr_handler, interrupt_handlers
        .global __masys_setup_idt, __masys_cli, __masys_sti
        .global __masys_userjmp, __masys_userleave

        .extern panic

intr_handler:
        .macro intrhead from, to
        .if (\to - \from)
        .align 8
        push    %eax                    # 1-byte opcode
        mov     $(\from), %al           # 2-byte opcode
        jmp     intr_common             # 5-byte or 2-byte opcode
        intrhead "(\from + 1)", \to
        .endif
        .endm
            intrhead 0 100
            intrhead 100 200
            intrhead 200 256

intr_common:
            movzx   %al, %eax
            pushal
            push    %eax
            lea     interrupt_handlers(, %eax, 4), %eax
            cmp     $0, %eax
            jz      intr_finish
            call    *(%eax)
intr_finish:
            add     $4, %esp
            popal
            pop     %eax
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
            push    $0x01D57ACC         # magic value "oldstack"
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

__masys_userleave:
            cli
            mov     $0x10, %ax          # set data segments to kernel ones
            mov     %ax, %ds
            mov     %ax, %es
            mov     %ax, %fs
            mov     %ax, %gs

            add     $20, %esp           # discard iret return pointers

            pop     %eax                # magic value
            cmp     $0x01D57ACC, %eax
            jne     panic
            pop     %eax                # address of TSS's esp0
            mov     (%eax), %ecx        # top of kernel stack
            pop     %edx                # top of the old interrupt stack
            mov     %edx, (%eax)        # use interrupt stack for interrupts
            pop     %edi
            pop     %esi
            pop     %ebx
            pop     %ebp

            sti
            ret                         # Return as if from __masys_userjmp

        .section .data
        .global interrupt_handlers
interrupt_handlers:
        .fill 256, 4, 0x0
