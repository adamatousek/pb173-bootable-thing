#define ASM_FILE        1

    .text
    .global alloca

alloca:
            pop     %ecx
            sub     (%esp), %esp
            mov     %esp, %eax
            sub     $4, %esp
            jmp     *%ecx
