#ifndef _MASYS_MEM_PAGING_HPP_
#define _MASYS_MEM_PAGING_HPP_

#include<types.hpp>

namespace masys {

const size_t PAGE_SIZE = 0x1000;
const size_t PAGEDIR_ENTRIES = 1024;
const u32 HIGHER_HALF = 0xC0000000;

namespace mem {

struct PageEntry {
    union {
        struct {
            u32 present : 1;
            u32 rw : 1;
            u32 user : 1;
            u32 wthru : 1;
            u32 nocache : 1;
            u32 accesed : 1;
            u32 dirty : 1;
            u32 big : 1;
            u32 global : 1;
            u32 custom : 3;
            u32 address_base : 20;
        };
        u32 _raw;
    };

    PageEntry() : _raw( 0 ) {};
};

} /* mem */
} /* masys */

#endif /* end of include guard: _MASYS_MEM_PAGING_HPP_ */
