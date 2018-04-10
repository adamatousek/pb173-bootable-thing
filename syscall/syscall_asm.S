#define ASM_FILE        1
/* Yes, ugly as shit and should be defined elsewhere in order not to repeat */
#define HIGHER_HALF     0xC0000000
#define N_SYSCALLS      32
#define ENOSYS          38
#define EINVAL          22

        .section .text
        .global __masys_syscall_handler
        .global syscall_fn_table, syscall_argc_table
        .global syscall

__masys_syscall_handler:
        cmp     $(N_SYSCALLS), %eax
        jge     enosys
        lea     syscall_fn_table(, %eax, 4), %edx
        cmp     $0, %edx
        je      enosys
        pusha
        mov     44(%esp), %ebx           # user %esp
        add     $4, %ebx                 # user %esp + 4 (argument address)
        js      badptr
        movzbl  syscall_argc_table(, %eax, 1), %ecx
        lea     (%ebx, %ecx, 4), %ebx    # after last argument
        cmp     $(HIGHER_HALF), %ebx
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
        call    *(%eax)
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
        mov     4(%esp), %eax
        int     $0xAD
        ret


        .section .data
syscall_fn_table:
        .align 4
        .fill N_SYSCALLS, 4, 0x0

syscall_argc_table:
        .fill N_SYSCALLS, 1, 0x0
