#include <dev/vga.hpp>
#include <dev/serial.hpp>
#include <mem/vmmap.hpp>
#include <mem/frames.hpp>
#include <mem/malloc.hpp>
#include <gdt.hpp>
#include <util.hpp>
#include <debug.hpp>
#include <interrupt.hpp>
#include <syscall/syscall.hpp>
#include <multiboot2.h>
#include <panic.hpp>
#include <userspace.hpp>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace masys {
namespace dbg {
dev::Vga *vga;
dev::SerialLine *ser;
} /* dbg */
namespace mem {
PageAllocator *page_allocator;
SubpageAllocator *allocator;
}
InterruptManager *intr;

extern u32 *tss;

void init_glue( mem::PageAllocator *, dev::SerialLine *, dev::Vga * );

void kernel( unsigned long magic, unsigned long addr )
{
    // Setup output devices
    dev::SerialLine ser( dev::SERIAL_PORT_1 );
    dev::Vga vga( (u8*) dev::Vga::MEMORY_MAPPED + HIGHER_HALF );
    vga.clear();

    // Link output devices as debug devices
    dbg::vga = &vga;
    dbg::ser = &ser;
    using dbg::vout;
    using dbg::sout;

    vga.puts( "\nSwitched.\n" );

    // Replace 4 MiB mapping with page table
    mem::remap_kernel_text();

    // Put frame allocator low on stack
    mem::FrameAllocator fal;

    sout() << "\nStrankovaci adresar: " << &mem::page_directory
           << "\n`-- fysicky: "
           << mem::PageAllocator::virt2phys( (u32) &mem::page_directory )
           << "\n`-- fysicky: "
           << mem::PageAllocator::virt2phys( 0xFFFFF000 )
           << "\nPriblizny vrchol zasobniku: " << &ser
           << "\n`-- fysicky: " << mem::PageAllocator::virt2phys( (u32) &ser )
           << "\nInformace od Multibootu: " << ( void* ) addr;

    // TODO: Don't assume that multiboot info is in lower 4M
    vga.puts( "\nCo nam Multiboot povedel:\n================================\n" );

    auto tag = reinterpret_cast< multiboot_tag * >( addr + 8 );

    while ( tag->type != MULTIBOOT_TAG_TYPE_END ) {
        if ( tag->type == MULTIBOOT_TAG_TYPE_MODULE ) {
            auto tm = reinterpret_cast< multiboot_tag_module * >( tag );
            vout() << "-- modul ( cmdline: \"" << tm->cmdline << "\") s obsahem:\n";

            for ( auto d = reinterpret_cast< const char * >( tm->mod_start );
                       d < reinterpret_cast< const char * >( tm->mod_end );
                       ++d )
                vga.putch( *d );
        } else if ( tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO ) {
            auto tm = reinterpret_cast< multiboot_tag_basic_meminfo * >( tag );
            vout() << "-- pamet\n `-- nizka: " << tm->mem_lower
                   << " KiB\n `- vysoka: " << tm->mem_upper << " KiB\n";
        } else if ( tag->type == MULTIBOOT_TAG_TYPE_MMAP ) {
            auto tm = reinterpret_cast< multiboot_tag_mmap * >( tag );
            vout() << "-- mapa pameti (vice na seriove lince)\n";
            fal.init( tm ); // Initialise the frame allocator
            ser.puts( "\nAlokator ramcu nastaven.\n" );
        }
        tag = reinterpret_cast< multiboot_tag * >(
                ( ( reinterpret_cast< unsigned long >( tag ))
                  + tag->size + 7 ) & 0xfffffff8UL );
    }

    /* Cancel id-mapping */
    mem::unmap_id_low();

    mem::PageAllocator pal( &fal );
    mem::SubpageAllocator spal( &pal );
    mem::page_allocator = &pal;
    mem::allocator = &spal;

    init_glue( &pal, &ser, &vga );

    const char *foo = "It's printf, bitch!";
    int fool = strlen( foo );

    printf("I have only %d characters to say: %s\n", fool, foo );
    fprintf(vgaout, "\nI have only %d characters to say: %s\n", fool, foo );

    //pal.map( fal.alloc(), 0xD000'0000 );

    auto syscall_stack_base = pal.alloc( 4 ) + 0x4000;
    setup_flat_gdt( syscall_stack_base );

    InterruptManager intman;
    intr = &intman;

    puts( "Interrupt manager set up." );
    setup_syscalls();
    puts( "Syscalls set up." );

    auto p1 = pal.alloc( 2 ),
         p2 = pal.alloc( 4 );

    *reinterpret_cast< char * >( p1 ) = 'X';
    *reinterpret_cast< char * >( p2 ) = 'Y';

    pal.free( p1, 2 );
    p1 = pal.alloc( 3 );
    auto p3 = pal.alloc( 1 );
    *reinterpret_cast< char * >( p3 ) = 'Z';
    *reinterpret_cast< char * >( p1 ) = 'W';

    /*
    pal.free( p1, 3 );
    pal.free( p2, 4 );
    pal.free( p3, 1 );
    */

    auto m1 = malloc( 8 );
    auto m2 = malloc( 127 );
    auto m3 = malloc( 16 );
    free( m1 );
    m1 = malloc( 4 );
    auto m1r = realloc( m1, 8 );
    free( m1r );
    free( m3 );
    free( m2 );

    /* setup a very basic userspace */
    auto u_stack = pal.alloc( 4,  /* user = */ true ) + 4 * PAGE_SIZE;
    auto u_text_virt = pal.find_available( 8, mem::reserved::USER_HEAP_BEGIN,
                                              mem::reserved::USER_HEAP_END );
    auto u_text_phys = mem::PageAllocator::virt2phys(
            reinterpret_cast< u32 >( hello_kernel ) );
    for( int i = 0; i < 8; ++i ) {
        pal.map( (i - 4) * PAGE_SIZE + u_text_phys & ~0xFFF,
                 i * PAGE_SIZE + u_text_virt, mem::PageEntry::DEFAULT_FLAGS_USER );
    }

    u_text_virt += 4 * PAGE_SIZE;
    u_text_virt |= u_text_phys & 0xFFF;

    sout() << "Vstupuji do userprostoru!\n";

    int exitval = userjmp( u_text_virt, u_stack, tss + 1 );

    printf( "Opoustim userprostor!\nNavratova hodnota = 0x%x\n", exitval );

#if 0
    vga.puts( "Nyni ocekavam vstup na seriove lince.\n" );
    vga.puts( "^D ukonci cinnost tohoto bohorovneho jadra.\n\n" );

    ser.puts( "Ahoj svete za seriovou linkou! Povez mi neco:\n" );

    const char CTRL_D = 4;

    unsigned char a;
    do {
        ser.getch( a );
        vga.putch( a );
        ser.putch( a );
    } while ( a != CTRL_D );
#elif 1
    vga.puts( "Stisknete klavesu Any k usmrceni jadra, 'p' vyvola paniku.\n" );
    ser.puts( "Stisknete klavesu Any k usmrceni jadra, 'p' vyvola paniku.\n" );
    unsigned char a;
    ser.getch( a );
    if ( a == 'p' )
        panic();
#endif

    ser.puts( "\nKernel konec.\n" );
    vga.puts( "\nKernel konec.\n" );
}

} /* masys */

extern "C" {
void main( unsigned long magic, unsigned long addr )
{
    masys::kernel( magic, addr );
}

void __cxa_pure_virtual()
{
    /* Unreachable */
}

}


