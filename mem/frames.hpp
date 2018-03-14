#ifndef _MASYS_MEM_FRAMES_HPP_
#define _MASYS_MEM_FRAMES_HPP_

#include <mem/paging.hpp>
#include <multiboot2.h>

namespace masys {
namespace mem {

// Tracking availability of at most 8 * 4096 frames
struct FrameSubAllocator {
    u32 base_addr = 0xFFFFFFFF;
    u8 *bitmap;
    u16 n_free = 0; // number of free frames
    u16 last = 0;   // number (bitmap index) of after the last allocated frame

    void clean( u16 );
    u32 alloc();
};

struct FrameSubAllocatorStatic : FrameSubAllocator {
    u8 bm[ 4 ];
    FrameSubAllocatorStatic() { bitmap = bm; }
};

struct MemoryArea {
    u32 base_addr = 0;
    u32 size = 0;
};

// The top-level allocator
class FrameAllocator {
    FrameSubAllocatorStatic sub_st; // Statically allocated bitmap for 32 pages
    FrameSubAllocator subs[ 32 ]; // Dynamically allocated bitmaps
    MemoryArea queued[ 8 ]; // Currently unmanaged available memory areas

    FrameSubAllocator * init_new_suballocator();

public:
    void init( const multiboot_tag_mmap * );
    u32 alloc();
    void free( u32 );
};

} /* mem */
} /* masys */

#endif /* end of include guard: _MASYS_MEM_FRAMES_HPP_ */
