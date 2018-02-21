#include "multiboot2.h"

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

    struct multiboot_tag *tag = addr + 8;

    while ( tag->type != MULTIBOOT_TAG_TYPE_END ) {
        puts( "-" );
        if ( tag->type == MULTIBOOT_TAG_TYPE_MODULE ) {
            struct multiboot_tag_module *tm = tag;
            puts( tm->cmdline );
        }
        tag = ( ( (unsigned long) tag ) + tag->size + 7 ) & 0xfffffff8UL;
    }
}
