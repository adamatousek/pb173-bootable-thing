#define ASM_FILE        1
#include "config.hpp"
// Yes, fuj.
#define ENOSYS          38
#define EINVAL          22

        .section .text
        .global __masys_syscall_handler
        .global syscall_fn_table, syscall_argc_table
        .global syscall

__masys_syscall_handler:
        sti
        cmp     $(MASYS_N_SYSCALLS), %eax
        jge     enosys
        mov     syscall_fn_table(, %eax, 4), %edx
        cmp     $0, %edx
        je      enosys
        pusha
        mov     44(%esp), %ebx           # user %esp
        cmp     $0, %ebx
        jle     badptr
        movzbl  syscall_argc_table(, %eax, 1), %ecx
        lea     (%ebx, %ecx, 4), %eax    # after last argument
        cmp     $(MASYS_HIGHER_HALF), %eax
        jl      badptr
        mov     %esp, %ebp
copyargs_beg:
        cmp     $0, %ecx
        je      copyargs_end
        dec     %ecx
        push    (%ebx, %ecx, 4)
        jmp     copyargs_beg
copyargs_end:
        mov     %edx, %eax
        xor     %edx, %edx
        call    *%eax
        mov     %ebp, %esp
        mov     %eax, 28(%esp)      # put syscall's low return to future eax
        mov     %edx, 20(%esp)      # put syscall's high return to future edx
        popa
        iret
enosys:
        mov     $(-ENOSYS), %eax
        iret
badptr:                             # probably just raise #GP?
        popa
        mov     $(-EINVAL), %eax
        iret

syscall:
        pop     %ebx
        pop     %eax
        int     $0xAD
        sub     $4, %esp
        push    %ebx
        ret

        .section .data
syscall_fn_table:
        .align 4
        .fill MASYS_N_SYSCALLS, 4, 0x0

syscall_argc_table:
        .fill MASYS_N_SYSCALLS, 1, 0x0

