#include <mem/frames.hpp>
#include <mem/alloca.hpp>
#include <panic.hpp>
#include <debug.hpp>

extern "C" {

extern void _kernel_start( void );
extern void _kernel_end( void );
extern void _bss_end( void );

}

namespace masys {
namespace mem {

void FrameAllocator::init( const multiboot_tag_mmap *mmap )
{
    /* Enqueue memory areas to be managed */
    // Yes, it is suboptimal if the available memory has holes in it, because
    // the bitmaps can currently only cover continuous area.
    for ( int i = 0, j = 0;
          i < (mmap->size - 8) / mmap->entry_size && j < 8;
          i++ )
    {
        auto & m = mmap->entries[ i ];
        switch ( m.type ) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                if ( m.addr > 0xFFFFFFFFul )
                    break;
                queued[ j ].base_addr = m.addr;
                if ( m.addr + m.len > 0xFFFFFFFFul )
                    queued[ j ].size = 0xFFFFFFFFul - m.addr;
                else
                    queued[ j ].size = m.len;
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
    int i = 0;
    while ( i < 8 && queued[ i ].size < 32 * PAGE_SIZE )
        ++i;
    if ( i == 8 )
    {
        dbg::sout() << "E: Frame allocator initialisation failed\n";
        panic();
    }

    dbg::sout() << "\nQueued memory areas:\n";
    for ( const auto & ma : queued ) {
        dbg::sout() << "- addr: " << dbg::hex() << ma.base_addr
                    << ", size: " << dbg::dec() << ma.size << '\n';
    }

    queued[ i ].size -= 32 * PAGE_SIZE;
    sub_st.n_free = 32;
    sub_st.base_addr = queued[ i ].base_addr;

    dbg::sout() << "\nQueued memory areas:\n";
    for ( const auto & ma : queued ) {
        dbg::sout() << "- addr: " << dbg::hex() << ma.base_addr
                    << ", size: " << dbg::dec() << ma.size << '\n';
    }
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
    panic();
}

FrameSubAllocator * FrameAllocator::init_new_suballocator()
{
    dbg::sout() << "Initializing new suballocator\n";
    MemoryArea *ma = nullptr;
    for ( int i = 0; i < 8; ++i ) {
        if ( queued[ i ].size >= PAGE_SIZE ) {
            ma = queued + i;
            break;
        }
    }

    if ( ! ma )
        return nullptr;

    dbg::sout() << "- using memory area {" << dbg::hex() << ma->base_addr
        << dbg::dec() <<", "<< ma->size << "}\n";

    const u32 maxsz = PAGE_SIZE * PAGE_SIZE * 8;
    u32 sz = ( ma->size < maxsz ) ? ( ma->size & ~0xFFF ) : maxsz;

    dbg::sout() << "- will init " << sz << " bytes\n";

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

    // TODO: proper virtual mapping, this is sooooo temporary and unsafe
    // However, I must be sure that there, in fact, *is* a page table to map
    // into, because the VM mapper must not allocate a new frame for it.
    newsub->bitmap = reinterpret_cast< u8* >( sub_st.alloc() + HIGHER_HALF );
    newsub->clean( sz / PAGE_SIZE );

    return newsub;
}

void FrameAllocator::free( u32 phys )
{
    const u32 maxsz = PAGE_SIZE * PAGE_SIZE * 8;
    FrameSubAllocator *sub = nullptr;
    for ( int i = 0; i < 32; ++i ) {
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

    dbg::sout() << "freeing frame, phys. 0x" << dbg::hex() << phys << '\n';
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
        if ( ++last == PAGE_SIZE * PAGE_SIZE * 8 )
            last = 0;
    } while ( used );

    bitmap[ bytei ] |= ( 0x80 >> biti );
    --n_free;

    dbg::sout() << " - suballoc: selecting frame " << ( last - 1 )
        << ", phys. 0x" << dbg::hex() << base_addr + ( last - 1 ) * PAGE_SIZE << '\n';
    return base_addr + ( last - 1 ) * PAGE_SIZE;
}

} /* mem */
} /* masys */
