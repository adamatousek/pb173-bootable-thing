#ifndef _MASYS_MEM_VMMAP_HPP_
#define _MASYS_MEM_VMMAP_HPP_

#include <mem/paging.hpp>

namespace masys {
namespace mem {

class PageDirectory {
    PageEntry pgdir[ PAGEDIR_ENTRIES ];

public:
    static bool is_mapped( u32 virt ) {
        auto pgdir_e = reinterpret_cast< PageEntry * >( 0xFFFF'F000 | ( virt >> 20 ) & ~0x3 );
        auto pgtbl_e = reinterpret_cast< PageEntry * >( 0xFFC0'0000 | ( virt >> 10 ) & ~0x3 );

        return pgdir_e->present && pgdir_e->big | pgtbl_e->present;
    }

    static u32 virt2phys( u32 virt ) {
        auto pgdir_e = reinterpret_cast< PageEntry * >( 0xFFFF'F000 | ( virt >> 20 ) & ~0x3 );
        if ( pgdir_e->big )
            return ( pgdir_e->address_base << 12 ) | ( virt & 0x003F'FFFF );

        auto pgtbl_e = reinterpret_cast< PageEntry * >( 0xFFC0'0000 | ( virt >> 10 ) & ~0x3 );
        return ( pgtbl_e->address_base << 12 ) | ( virt & 0xFFF );
    }
};

static_assert( sizeof( PageDirectory ) == PAGE_SIZE, "Page Directory has wrong size" );

extern "C" {

extern masys::mem::PageDirectory page_directory;

}
} /* mem */
} /* masys */



#endif /* end of include guard: _MASYS_MEM_VMMAP_HPP_ */
