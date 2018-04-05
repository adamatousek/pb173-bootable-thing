#define ASM_FILE        1

        .section .text
        .global intr_handler, interrupt_handlers
        .global __masys_setup_idt, __masys_cli, __masys_sti
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

        .section .data
        .global interrupt_handlers
interrupt_handlers:
        .fill 256, 4, 0x0
