#include <mem/frames.hpp>
#include <mem/alloca.hpp>
#include <panic.hpp>
#include <debug.hpp>
#include <types.hpp>

extern "C" {

extern char _kernel_start;
extern char _kernel_end;
extern char _bss_end;

extern masys::mem::PageTable kernel_pgtbl;

}

#define MASYS_VERBOSE_FALLOC 1

namespace masys {
namespace mem {

void FrameAllocator::init( const multiboot_tag_mmap *mmap )
{
    auto kernel_start = reinterpret_cast< u32 >( &_kernel_start ) - HIGHER_HALF;
    auto bss_end = reinterpret_cast< u32 >( &_bss_end ) - HIGHER_HALF;
    kernel_start -= kernel_start % PAGE_SIZE;
    bss_end += PAGE_SIZE - 1;
    bss_end -= bss_end % PAGE_SIZE;
    /* Enqueue memory areas to be managed */
    // Yes, it is suboptimal if the available memory has holes in it, because
    // the bitmaps can currently only cover continuous area.

    for ( int i = 0, j = 0;
          i < (mmap->size - 8) / mmap->entry_size && j < 8;
          i++ )
    {
        auto m = mmap->entries[ i ];
        auto & ma = queued[ j ];
        switch ( m.type ) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                if ( m.addr > 0xFFFFFFFFul )
                    break;
                if ( m.addr % PAGE_SIZE ) {
                    auto under = PAGE_SIZE - m.addr % PAGE_SIZE;
                    m.addr += under;
                    m.len -= under;
                }

                m.len -= m.len % PAGE_SIZE;

                ma.base_addr = m.addr;
                if ( m.addr + m.len > 0xFFFFFFFFul )
                    ma.size = 0xFFFFFFFFul - m.addr;
                else
                    ma.size = m.len;

                // Subtract memory occupied by kernel
                if ( kernel_start >= ma.base_addr &&
                     kernel_start < ma.base_addr + ma.size )
                {
                    auto newsz = kernel_start - ma.base_addr;
                    if ( newsz > 0 ) {
                        auto oldsz = ma.size;
                        ma.size = newsz;
                        ++j;
                        ma = queued[ j ];
                        if ( j < 8 ) {
                            ma.base_addr = m.addr;
                            ma.size = oldsz;
                        } else
                            break;
                    }
                }
                if ( bss_end >= ma.base_addr &&
                     bss_end < ma.base_addr + ma.size )
                {
                    ma.size -= bss_end - ma.base_addr;
                    ma.base_addr = bss_end;
                }

                if ( ma.size > 0 )
                    ++j;

                break;
            case MULTIBOOT_MEMORY_RESERVED:
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
            case MULTIBOOT_MEMORY_NVS:
            case MULTIBOOT_MEMORY_BADRAM:
            default:
                break;
        }
    }

    /* Find continuous 32 pages for the static suballocator */
    // Although I need full 32 pages, they might not be all used for bitmaps.
    // The continuity request is only for convenience.
    int i = 0;
    while ( i < 8 && queued[ i ].size < 32 * PAGE_SIZE )
        ++i;
    if ( i == 8 )
    {
        dbg::sout() << "E: Frame allocator initialisation failed\n";
        panic();
    }

#if MASYS_VERBOSE_FALLOC
    dbg::sout() << "\nQueued memory areas:\n";
    for ( const auto & ma : queued ) {
        dbg::sout() << "- addr: " << dbg::hex() << ma.base_addr
                    << ", size: " << dbg::dec() << ma.size << '\n';
    }
#endif

    queued[ i ].size -= 32 * PAGE_SIZE;
    sub_st.n_free = 32;
    sub_st.base_addr = queued[ i ].base_addr + queued[ i ].size;
}

u32 FrameAllocator::alloc()
{
    FrameSubAllocator *sub = nullptr;
    for ( int i = 0; i < 32; ++i ) {
        if ( subs[ i ].n_free > 0 ) {
            sub = subs + i;
            break;
        }
    }

    if ( ! sub )
        sub = init_new_suballocator();

    if ( sub )
        return sub->alloc();

    // TODO: hand out the pages from static suballocator last
    dbg::sout() << "No more free frames!\n";
    panic();
}

FrameSubAllocator * FrameAllocator::init_new_suballocator()
{
#if MASYS_VERBOSE_FALLOC
    dbg::sout() << "Initializing new suballocator\n";
#endif
    MemoryArea *ma = nullptr;
    for ( int i = 0; i < 8; ++i ) {
        if ( queued[ i ].size >= PAGE_SIZE ) {
            ma = queued + i;
            break;
        }
    }

    if ( ! ma )
        return nullptr;

#if MASYS_VERBOSE_FALLOC
    dbg::sout() << "- using memory area {addr: 0x" << dbg::hex()
        << ma->base_addr <<", sz: 0x"<< ma->size << "}\n";
#endif

    const u32 maxsz = PAGE_SIZE * PAGE_SIZE * 8;
    u32 sz = ( ma->size < maxsz ) ? ( ma->size & ~0xFFF ) : maxsz;

#if MASYS_VERBOSE_FALLOC
    dbg::sout() << "- popping 0x" << dbg::hex() << sz << " bytes\n";
#endif

    FrameSubAllocator *newsub = nullptr;
    for ( int i = 0; i < 32; ++i ) {
        if ( subs[ i ].base_addr == 0xFFFFFFFF ) {
            newsub = subs + i;
            break;
        }
    }

    if ( ! newsub )
        return nullptr;

    ma->size -= sz;
    newsub->n_free = sz / PAGE_SIZE;
    newsub->base_addr = ma->base_addr + ma->size;

    auto bmphys = sub_st.alloc();
    auto bmvirt = ( 31 - sub_st.n_free ) * PAGE_SIZE +
                  reserved::FRAME_ALLOC_BITMAP_START;
#if MASYS_VERBOSE_FALLOC
    dbg::sout() << " - new bitmap phys: 0x" << dbg::hex() << bmphys
                << ", virt: 0x" << bmvirt << '\n';
#endif
    kernel_pgtbl[ ( bmvirt >> 12 ) & 0x3FF ]._raw = bmphys | 0x103;
    newsub->bitmap = reinterpret_cast< u8* >( bmvirt );
    newsub->clean( sz / PAGE_SIZE );

    return newsub;
}

void FrameAllocator::free( u32 phys )
{
    const u32 maxsz = PAGE_SIZE * PAGE_SIZE * 8;
    FrameSubAllocator *sub = nullptr;
    for ( int i = 31; i >= 0; --i ) {
        if ( subs[ i ].base_addr <= phys &&
             phys < subs[ i ].base_addr + maxsz )
        {
            sub = subs + i;
            break;
        }
    }

    if ( ! sub )
        panic();

    u16 pgi = ( phys - sub->base_addr ) / PAGE_SIZE;
    u16 bytei = pgi / 8;
    u8 biti = pgi % 8;
    sub->bitmap[ bytei ] &= ~( 0x80 >> biti );
    sub->n_free++;

#if MASYS_VERBOSE_FALLOC
    dbg::sout() << " - freeing frame, phys. 0x" << dbg::hex() << phys << '\n';
#endif
}

void FrameSubAllocator::clean( u16 npgs )
{
    u8 *b = bitmap;

    while ( npgs >= 8 ) {
        *b++ = 0x00;
        npgs -= 8;
    }

    if ( npgs )
        *b++ = 0xFF >> npgs;

    while ( b < bitmap + PAGE_SIZE )
        *b++ = 0xFF;
}

u32 FrameSubAllocator::alloc()
{
    u16 bytei;
    u8 biti;
    bool used;

    do {
        bytei = last / 8;
        biti = last % 8;
        used = bitmap[ bytei ] & ( 0x80 >> biti );
        if ( ++last == PAGE_SIZE * 8 )
            last = 0;
    } while ( used );

    bitmap[ bytei ] |= ( 0x80 >> biti );
    --n_free;

#if MASYS_VERBOSE_FALLOC
    dbg::sout() << " - suballoc: selecting frame " << ( last - 1 )
        << ", phys. 0x" << dbg::hex() << base_addr + ( last - 1 ) * PAGE_SIZE << '\n';
#endif
    return base_addr + ( last - 1 ) * PAGE_SIZE;
}

} /* mem */
} /* masys */
