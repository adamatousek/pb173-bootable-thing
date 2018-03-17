#define ASM_FILE        1

    .text
    .global __masys_invalidate_page, __masys_flush_tlb
__masys_invalidate_page:
            mov         4(%esp), %eax
            invlpg      (%eax)
            ret

__masys_flush_tlb:
            mov         %cr3, %eax
            mov         %eax, %cr3
            ret
