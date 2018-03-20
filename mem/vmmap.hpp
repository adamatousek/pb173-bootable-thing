#ifndef _MASYS_MEM_VMMAP_HPP_
#define _MASYS_MEM_VMMAP_HPP_

#include <mem/paging.hpp>

namespace masys {
namespace mem {

class FrameAllocator;

class PageAllocator {
    FrameAllocator *fal;

public:
    PageAllocator( FrameAllocator *fa )
        : fal( fa ) {}
    u32 alloc( u16 pages, bool user = false );
    void free ( u32 virt, u16 pages );
    void map( u32 phys, u32 virt, u32 flags = PageEntry::DEFAULT_FLAGS_KERNEL );
    static void unmap( u32 virt );
    static u32 find_available( u16 pages, u32 from, u32 to );
    static bool is_mapped( u32 virt );
    static u32 virt2phys( u32 virt );
};

// Replaces the 4 MiB mapping with a page table
void remap_kernel_text();

// Removes id-mapping of low 4 MiB
void unmap_id_low();

} /* mem */
} /* masys */



#endif /* end of include guard: _MASYS_MEM_VMMAP_HPP_ */
