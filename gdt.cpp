#include "gdt.hpp"
#include "types.hpp"
#include "mem/vmmap.hpp"
#include <new>

namespace masys {

extern "C" {
// Defined in mem/paging.S
void __masys_setup_gdt( u32 sz, void *gdt );
}

void setup_flat_gdt( unsigned syscall_stack )
{
    u32 gdtsz = 6 * 2;
    u32 *gdt = new u32[ gdtsz ];

    gdtsz *= 4;
    --gdtsz;

    // Null descriptor
    gdt[ 0 ] = 0x0000'0000;
    gdt[ 1 ] = 0x0000'0000;

    // Kernel code descriptor
    gdt[ 2 ] = 0x0000'ffff;
    gdt[ 3 ] = 0x00cf'9a00;

    // Kernel data descriptor
    gdt[ 4 ] = 0x0000'ffff;
    gdt[ 5 ] = 0x00cf'9200;

    // User code descriptor
    gdt[ 6 ] = 0x0000'ffff;
    //gdt[ 7 ] = 0x00cb'fa00;
    gdt[ 7 ] = 0x00cf'fa00;

    // User data descriptor
    gdt[ 8 ] = 0x0000'ffff;
    //gdt[ 9 ] = 0x00cb'f200;
    gdt[ 9 ] = 0x00cf'f200;

    u32 *tss = new u32[ 26 ];
    tss[ 1 ] = syscall_stack;// ESP0
    tss[ 2 ] = 0x10; // SS0
    tss[ 25 ] = 0x0068'0000; // IOPB

    // TSS descriptor
    auto tssi = reinterpret_cast< u32 >( tss );
    gdt[ 10 ] = 0x0000'0068;
    gdt[ 11 ] = 0x0040'8900;
    gdt[ 10 ] |= tssi << 16;
    gdt[ 11 ] |= ( tssi >> 16 ) & 0xFF;
    gdt[ 11 ] |= tssi & 0xFF00'0000;

    __masys_setup_gdt( gdtsz, gdt );
}

} /* masys */
