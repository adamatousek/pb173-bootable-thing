#define ASM_FILE        1

    .text
    .globl __masys_inb, __masys_outb

__masys_inb:
    xor     %eax, %eax
    mov     4(%esp), %edx
    in      %dx, %al
    ret

__masys_outb:
    mov     4(%esp), %edx
    mov     8(%esp), %eax
    out     %al, %dx
    ret
