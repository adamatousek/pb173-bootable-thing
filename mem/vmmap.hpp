#ifndef _MASYS_MEM_VMMAP_HPP_
#define _MASYS_MEM_VMMAP_HPP_

#include <mem/paging.hpp>
#include <mem/frames.hpp>

namespace masys {
namespace mem {

class PageAllocator {
    FrameAllocator *fal;

public:
    PageAllocator( FrameAllocator *fa )
        : fal( fa ) {}
    u32 alloc( u16 pages );
    void free ( u32 virt, u16 pages );
    void map( u32 phys, u32 virt, u32 flags = 0x103 );
    void unmap( u32 virt );
    static u32 find_available( u16 pages );
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
