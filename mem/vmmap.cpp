#include <mem/vmmap.hpp>
#include <mem/frames.hpp>
#include <types.hpp>
#include <debug.hpp>
#include <dev/vga.hpp>

extern "C" {

extern char _kernel_start;
extern char _kernel_end;
extern char _bss_end;
extern masys::mem::PageTable kernel_pgtbl;

void __masys_invalidate_page( masys::u32 virt );
void __masys_flush_tlb();

}

#define MASYS_VERBOSE_PALLOC 0

namespace masys {
namespace mem {

namespace {
    PageEntry * const pgdir = reinterpret_cast< PageEntry * >(0xFFFF'F000);
    constexpr PageEntry & pgdir_e( u32 virt )
    {
        return *reinterpret_cast< PageEntry * >( 0xFFFF'F000 | ( virt >> 20 ) & ~0x3 );
    }
    constexpr PageEntry * pgtbl( u32 virt )
    {
        return reinterpret_cast< PageEntry * >( 0xFFC0'0000 | ( virt >> 10 ) & ~0xFFF );
    }
    constexpr PageEntry & pgtbl_e( u32 virt )
    {
        return *reinterpret_cast< PageEntry * >( 0xFFC0'0000 | ( virt >> 10 ) & ~0x3 );
    }
}

u32 PageAllocator::alloc( u16 pages, bool user )
{
#if MASYS_VERBOSE_PALLOC
    dbg::sout() << "Allocating " << pages << ( user ? " user" : " kernel" )
                << " pages...\n";
#endif
    u32 begin = find_available( pages,
            user ? reserved::USER_HEAP_BEGIN : reserved::HEAP_BEGIN,
            user ? reserved::USER_HEAP_END : reserved::HEAP_END );
    if ( begin ) {
        for ( int i = 0; i < pages; ++i ) {
            map( fal->alloc(), begin + i * PAGE_SIZE,
                 user ? PageEntry::DEFAULT_FLAGS_USER
                      : PageEntry::DEFAULT_FLAGS_KERNEL );
        }
    }
    return begin;
}

void PageAllocator::free( u32 virt, u16 pages )
{
    if ( virt == 0 )
        return;
#if MASYS_VERBOSE_PALLOC
    dbg::sout() << "Freeing " << pages << " pages from 0x" << dbg::hex()
                << virt << "...\n";
#endif
    while ( pages > 0 ) {
        u32 phys = pgtbl_e( virt ).address_base << 12;
        unmap( virt );
        fal->free( phys );
        --pages;
        virt += PAGE_SIZE;
    }

}

void PageAllocator::map( u32 phys, u32 virt, u32 flags )
{
    auto pgdir_i = virt >> 22;
    if ( ! pgdir[ pgdir_i ].present ) {
        auto newphys = fal->alloc();
        pgdir[ pgdir_i ]._raw = newphys | flags;
        for ( int i = 0; i < PAGEDIR_ENTRIES; ++i )
            pgtbl( virt )[ i ]._raw = 0;
#if MASYS_VERBOSE_PALLOC
        dbg::sout() << dbg::hex() << " - created new page table at Px"
                    << newphys << '\n';
#endif
    }
    pgtbl_e( virt )._raw = phys | flags;

#if MASYS_VERBOSE_PALLOC
    dbg::sout() << dbg::hex() << " - mapped Vx" << virt
                << " -> Px" << phys << '\n';
#endif
}

void PageAllocator::unmap( u32 virt )
{
    pgtbl_e( virt ).present = 0;
    __masys_invalidate_page( virt );
#if MASYS_VERBOSE_PALLOC
    dbg::sout() << dbg::hex() << " - unmapped Vx" << virt << " (-> Px"
                << ( pgtbl_e( virt ).address_base << 12 ) << ")\n";
#endif
}

u32 PageAllocator::find_available( u16 pages, u32 from, u32 to )
{
    u32 pg = from;
    u32 begin = 0;
    int free = 0;
    while ( free < pages && begin < to ) {
        if ( free == 0 )
            begin = pg;
        if ( ! pgdir_e( pg ).present )
        {
            free += PAGEDIR_ENTRIES;
            pg += PAGEDIR_ENTRIES * PAGE_SIZE;
        } else if ( pgdir_e( pg ).big ) {
            free = 0;
            pg += PAGEDIR_ENTRIES * PAGE_SIZE;
        } else {
            if ( pgtbl_e( pg ).present )
                free = 0;
            else
                ++free;
            pg += PAGE_SIZE;
        }
    }

    if ( begin >= to )
    {
        dbg::sout() << "E: Couldn't find " << pages << " continuous pages!\n";
        return 0;
    }

#if MASYS_VERBOSE_PALLOC
    dbg::sout() << " - found " << pages << " continuous pages from 0x"
                << dbg::hex() << begin << '\n';
#endif

    return begin;
}

bool PageAllocator::is_mapped( u32 virt )
{
    return pgdir_e( virt ).present && pgdir_e( virt ).big ||
                                      pgtbl_e( virt ).present;
}

u32 PageAllocator::virt2phys( u32 virt )
{
    if ( pgdir_e( virt ).big )
        return ( pgdir_e( virt ).address_base << 12 ) | ( virt & 0x003F'FFFF );
    return ( pgtbl_e( virt ).address_base << 12 ) | ( virt & 0xFFF );
}

void remap_kernel_text()
{
    auto kernel_start = reinterpret_cast< u32 >( &_kernel_start ) - HIGHER_HALF;
    auto bss_end = reinterpret_cast< u32 >( &_bss_end ) - HIGHER_HALF;

    for ( int i = kernel_start / PAGE_SIZE;
          i < ( bss_end + PAGE_SIZE - 1 ) / PAGE_SIZE;
          ++i )
    {
        PageEntry e;
        e._raw = i * PAGE_SIZE;
        e.present = 1;
        e.rw = 1;
        e.global = 1;
        kernel_pgtbl[ i ] = e;
    }
    PageEntry newent;
    newent._raw = dev::Vga::MEMORY_MAPPED;
    newent.present = 1;
    newent.rw = 1;
    newent.global = 1;
    kernel_pgtbl[ dev::Vga::MEMORY_MAPPED >> 12 ] = newent;

    newent._raw = reinterpret_cast< u32 >( &kernel_pgtbl ) - HIGHER_HALF;
    newent.present = 1;
    newent.rw = 1;
    newent.global = 1;
    page_directory[ HIGHER_HALF >> 22 ] = newent;
    __masys_flush_tlb();
}

void unmap_id_low()
{
    page_directory[ 0 ]._raw = 0;
    __masys_flush_tlb();
}

} /* mem */
} /* masys */
