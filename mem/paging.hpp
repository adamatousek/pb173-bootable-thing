#ifndef _MASYS_MEM_PAGING_HPP_
#define _MASYS_MEM_PAGING_HPP_

/* Mapping of kernel memory (defined in config.hpp)
 * 0xC000'0000 - 0xC000'0FFF -- reserved for the heck of it
 * 0xC000'1000 - 0xC000'101F -- reserved for bitmaps of frame allocator
 * 0xC00B'8000 - 0xC00B'9000 -- memory mapped VGA text
 * 0xC010'0000 - ?           -- kernel text (at most 3 MiB)
 * 0xC040'0000 - ?           -- kernel heap
 *           ? - 0xFFBF'FFFF -- kernel stack
 * 0xFFC0'0000               -- current page directory
 */

#include <types.hpp>
#include <config.hpp>

namespace masys {

namespace mem {

struct PageEntry {
    union alignas( 4 ) {
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

    static const u32 DEFAULT_FLAGS_KERNEL = 0x103; // Global, RW, Present
    static const u32 DEFAULT_FLAGS_USER = 0x07; // User, RW, Present

    PageEntry() : _raw( 0 ) {};
};

static_assert( sizeof( PageEntry ) == 4,
               "PageEntry has wrong size" );
static_assert( alignof( PageEntry ) == 4,
               "PageEntry has wrong alignment" );

using PageTable = PageEntry [ PAGEDIR_ENTRIES ];

static_assert( sizeof( PageTable ) == PAGE_SIZE,
               "PageTable has wrong size" );

extern "C" {

extern masys::mem::PageTable page_directory;

}

} /* mem */
} /* masys */

#endif /* end of include guard: _MASYS_MEM_PAGING_HPP_ */
