#ifndef _MASYS_MEM_PAGING_HPP_
#define _MASYS_MEM_PAGING_HPP_

#include<types.hpp>

extern "C" {

void __masys_enable_paging( void *pgdir );

}

namespace masys {
namespace mem {

struct PageEntry {
    union {
        struct {
            u32 present : 1;
            u32 rw : 1;
            u32 wthru : 1;
            u32 nocache : 1;
            u32 accesed : 1;
            u32 big : 1;
            u32 dirty : 1;
            u32 global : 1;
            u32 custom : 2;
            u32 address_base : 12;
        };
        u32 _raw;
    };

    PageEntry() : _raw( 0 ) {};
};

void enable_paging( void *pgdir )
{
    __masys_enable_paging( pgdir );
}

} /* mem */
} /* masys */

#endif /* end of include guard: _MASYS_MEM_PAGING_HPP_ */
