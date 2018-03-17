#ifndef _MASYS_MEM_PAGING_HPP_
#define _MASYS_MEM_PAGING_HPP_

/* Mapping of kernel memory
 * 0xC000'0000 - 0xC000'0fff -- reserved for the heck of it
 * 0xC000'1000 - 0xC000'101f -- reserved for bitmaps of frame allocator
 * 0xC00B'8000 - 0xC00B'9000 -- memory mapped VGA text
 * 0xC010'0000 - ?           -- kernel text (at most 3 MiB)
 * 0xC040'0000 - ?           -- kernel heap
 *           ? - 0xFFBF'FFFF -- kernel stack
 * 0xFFC0'0000               -- current page directory
 */

#include<types.hpp>

namespace masys {

const size_t PAGE_SIZE = 0x1000;
const size_t PAGEDIR_ENTRIES = 1024;
const u32 HIGHER_HALF = 0xC0000000;

namespace mem {

namespace reserved {
const u32 FRAME_ALLOC_BITMAP_START = HIGHER_HALF + 0x1000;
const u32 HEAP_BEGIN = HIGHER_HALF + 0x400000;
} /* reserved */

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

using PageTable alignas( PAGE_SIZE ) = PageEntry [ PAGEDIR_ENTRIES ];

static_assert( sizeof( PageTable ) == PAGE_SIZE,
               "PageTable has wrong size" );
static_assert( alignof( PageTable ) == PAGE_SIZE,
               "PageTable has wrong alignment" );


extern "C" {

extern masys::mem::PageTable page_directory;

}

} /* mem */
} /* masys */

#endif /* end of include guard: _MASYS_MEM_PAGING_HPP_ */
