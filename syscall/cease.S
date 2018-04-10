# Syscall nr. 0 - cease()
# It's like exit(), but sounds finer.

            .section .text
            .global __masys_sys_cease

            .extern panic

__masys_sys_cease:
            cli
            mov     $0x10, %ax          # set data segments to kernel ones
            mov     %ax, %ds
            mov     %ax, %es
            mov     %ax, %fs
            mov     %ax, %gs

            add     $4, %esp            # discarding: return pointer
            pop     %eax
            add     $52, %esp           # discarding: pusha (8 * 4)
                                        #             info for iret (5 * 4)

            pop     %ecx                # magic value
            cmp     $0x01D57ACC, %ecx
            jne     panic
            pop     %ecx                # address of TSS's esp0
            pop     %edx                # top of the old interrupt stack
            mov     %edx, (%ecx)        # use interrupt stack for interrupts
            pop     %edi
            pop     %esi
            pop     %ebx
            pop     %ebp

            sti
            ret                         # Return as if from __masys_userjmp
