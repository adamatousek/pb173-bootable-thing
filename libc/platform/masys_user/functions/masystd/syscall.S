#define ASM_FILE        1
        .global syscall

syscall:
        pop     %ebx
        pop     %eax
        int     $0xAD
        sub     $4, %esp
        jmp     *%ebx

