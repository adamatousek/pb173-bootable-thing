#define ASM_FILE        1

    .text
    .globl __masys_enable_paging

__masys_enable_paging:
    mov     4(%esp), %eax
    mov     %eax, %cr3
    mov     %cr1, %eax
    or      $0x80000001, %eax
    mov     %eax, %cr1
