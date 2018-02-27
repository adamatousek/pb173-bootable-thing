#include "multiboot2.h"

extern "C" {

void main( unsigned long, unsigned long );

static volatile unsigned char *video = ( unsigned char * ) 0xB8000;

static void clear( void )
{
    for ( int i = 0; i < 80 * 24 * 2; i ++ )
        video[ i ] = 0;
}

static void putchar( int c )
{
    const int columns = 80;
    const int lines = 24;
    static int x = 0, y = 0;

    if ( c == '\b' && x > 0 ) {
        --x;
        return;
    }
    if ( c == '\r' || c == '\b' ) {
        x = 0;
        return;
    }

    if ( c == '\n' || x >= columns )
        x = 0, y++;

    if ( c == '\n' )
        return;

    if ( y >= lines )
        y = 0;

    int idx = ( x + y * columns ) * 2;
    video[ idx ] = c & 0xff;
    video[ idx + 1 ] = 0x07; /* vga attribute */

    ++ x;
}

void puts( const char *str )
{
    do putchar( *str ); while ( *str++ );
    putchar( '\n' );
}

/****************************************************************************/

// Defined in io.S
// All the ints are actually supposed to be chars
void outb( unsigned port, unsigned data );
unsigned inb( unsigned port );
}

const unsigned SER_PORT = 0x3f8;

void ser_init()
{
    outb(SER_PORT + 1, 0x00);    // Disable all interrupts
    outb(SER_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(SER_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(SER_PORT + 1, 0x00);    //                  (hi byte)
    outb(SER_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(SER_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(SER_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void ser_write( char data )
{
    while ( ( inb(SER_PORT + 5) & 0x20 ) == 0 )
        ;
    outb( SER_PORT, data );
}

char ser_read()
{
    while ( ( inb(SER_PORT + 5) & 0x01 ) == 0 )
        ;
    return inb( SER_PORT );
}

void ser_write( const char *data, unsigned sz )
{
    for ( int i = 0; i < sz; ++i )
        ser_write( *data++ );
}

void ser_read( char *data, unsigned sz )
{
    for ( int i = 0; i < sz; ++i )
        *data++ = ser_read();
}

/****************************************************************************/

void main( unsigned long magic, unsigned long addr )
{
    clear();

    if ( magic != MULTIBOOT2_BOOTLOADER_MAGIC )
    {
        puts( "invalid magic number :-(" );
        return;
    }

    if ( addr & 7 )
    {
        puts( "unaligned mbi :-(" );
        return;
    }

    puts( "Mame tyto Multiboot moduly:" );

    auto tag = reinterpret_cast< multiboot_tag * >( addr + 8 );

    while ( tag->type != MULTIBOOT_TAG_TYPE_END ) {
        puts( "-" );
        if ( tag->type == MULTIBOOT_TAG_TYPE_MODULE ) {
            auto tm = reinterpret_cast< multiboot_tag_module * >( tag );
            puts( tm->cmdline );
            for ( auto d = reinterpret_cast< const char * >( tm->mod_start );
                       d < reinterpret_cast< const char * >( tm->mod_end );
                       ++d )
                putchar( *d );
        }
        tag = reinterpret_cast< multiboot_tag * >(
                ( ( reinterpret_cast< unsigned long >( tag ))
                  + tag->size + 7 ) & 0xfffffff8UL );
    }

    puts( "Nyni ocekavam vstup na seriove lince (coz je stdin, ehm)." );
    puts( "^D ukonci cinnost tohoto bohorovneho jadra.\n" );

    ser_init();

    for ( auto c : "Ahoj, svete.\n" )
        ser_write( c );

    const char CTRL_D = 4;

    char a;
    do {
        a = ser_read();
        putchar( a );
        ser_write( a );
    } while ( a != CTRL_D );

    puts( "\nKernel konec." );
}
