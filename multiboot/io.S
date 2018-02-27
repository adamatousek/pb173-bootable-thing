#define ASM_FILE        1

    .text
    .globl inb, outb

inb:
    xor     %eax, %eax
    mov     4(%esp), %edx
    in      %dx, %al
    ret

outb:
    mov     4(%esp), %edx
    mov     8(%esp), %eax
    out     %al, %dx
    ret
