#include <dev/vga.hpp>
#include <dev/serial.hpp>
#include <mem/paging.hpp>
#include <multiboot2.h>

namespace masys {

const size_t PAGE_SIZE = 4096;

alignas( PAGE_SIZE ) mem::PageEntry page_directory[ PAGE_SIZE ];

void kernel( unsigned long magic, unsigned long addr )
{
    dev::Vga vga;
    vga.clear();

    vga.puts( "Enabling paging...\n" );

    for ( auto & pgdir_entry : page_directory )
    {
        pgdir_entry._raw = 0;
    }
    auto & mypage = page_directory[ 1 << 8 ];
    mypage.present = 1;

    mem::enable_paging( page_directory );

    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC )
    {
        vga.puts( "invalid magic number :-(" );
        return;
    }

    if ( addr & 7 )
    {
        vga.puts( "unaligned mbi :-(" );
        return;
    }

    vga.puts( "Co nam Multiboot povedel:\n================================\n" );

    auto tag = reinterpret_cast< multiboot_tag * >( addr + 8 );

    while ( tag->type != MULTIBOOT_TAG_TYPE_END ) {
        if ( tag->type == MULTIBOOT_TAG_TYPE_MODULE ) {
            auto tm = reinterpret_cast< multiboot_tag_module * >( tag );
            vga.puts( "-- modul ( cmdline: \"" );
            vga.puts( tm->cmdline );
            vga.puts( "\") s obsahem:\n" );

            for ( auto d = reinterpret_cast< const char * >( tm->mod_start );
                       d < reinterpret_cast< const char * >( tm->mod_end );
                       ++d )
                vga.putch( *d );
        }
        tag = reinterpret_cast< multiboot_tag * >(
                ( ( reinterpret_cast< unsigned long >( tag ))
                  + tag->size + 7 ) & 0xfffffff8UL );
    }

    vga.puts( "Nyni ocekavam vstup na seriove lince (coz je stdin, ehm).\n" );
    vga.puts( "^D ukonci cinnost tohoto bohorovneho jadra.\n\n" );


    dev::SerialLine ser( dev::SERIAL_PORT_1 );
    ser.puts( "Ahoj svete za seriovou linkou! Povez mi neco:\n" );

    const char CTRL_D = 4;

    unsigned char a;
    do {
        ser.getch( a );
        vga.putch( a );
        ser.putch( a );
    } while ( a != CTRL_D );

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


