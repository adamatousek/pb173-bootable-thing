#include <dev/vga.hpp>
#include <dev/serial.hpp>
#include <mem/paging.hpp>
#include <util.hpp>
#include <multiboot2.h>

extern "C" {

extern masys::mem::PageEntry page_directory[];

}

namespace masys {

const size_t PAGE_SIZE = 0x1000;
const size_t PAGEDIR_ENTRIES = 1024;
const u32 HIGHER_HALF = 0xC0000000;

void kernel( unsigned long magic, unsigned long addr )
{
    dev::SerialLine ser( dev::SERIAL_PORT_1 );

    char buf[32];
    ser.puts( "Page Directory address: 0x" );
    itoa( (u32) page_directory, buf, 16 );
    ser.puts( buf );
    ser.putch( '\n' );
    ser.puts( "Stack pointer: 0x" );
    itoa( (u32) buf, buf, 16 );
    ser.puts( buf );
    ser.putch( '\n' );
    ser.puts( "Multiboot info address: 0x" );
    itoa( addr, buf, 16 );
    ser.puts( buf );
    ser.putch( '\n' );

    dev::Vga vga( (u8*) dev::Vga::MEMORY_MAPPED + HIGHER_HALF );
    vga.clear();
    vga.puts( "\nSwitched.\n" );

    // TODO: Don't assume that multiboot info is in lower 4M
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

    /* Cancel id-mapping */
    //page_directory[ 0 ].present = 0;

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


