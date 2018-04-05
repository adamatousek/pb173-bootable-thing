#define ASM_FILE        1

    .text
    .global __masys_invalidate_page, __masys_flush_tlb
    .global __masys_setup_gdt
__masys_invalidate_page:
            mov         4(%esp), %eax
            invlpg      (%eax)
            ret

__masys_flush_tlb:
            mov         %cr3, %eax
            mov         %eax, %cr3
            ret

__masys_setup_gdt:
            movw        4(%esp), %ax
            movw        %ax, 6(%esp)
            lgdt        6(%esp)
            ljmp        $0x8, $(dummy_for_cs_reload)
    dummy_for_cs_reload:
            mov         $0x10, %ax
            mov         %ax, %ds
            mov         %ax, %es
            mov         %ax, %fs
            mov         %ax, %gs
            mov         %ax, %ss
            mov         $0x2B, %ax
            ltr         %ax
            ret

