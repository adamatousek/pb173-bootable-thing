#ifndef _MASYS_DEV_VGA_H_
#define _MASYS_DEV_VGA_H_

#include <dev/output.hpp>

namespace masys {
namespace dev {

class Vga : public CharacterOutput {

    u8 * const video;
    const int columns = 80;
    const int lines = 24;

    u8 x = 0,
       y = 0,
       flags = 0x07;
public:
    static const u32 MEMORY_MAPPED = 0xB8000;

    Vga( u8 * fb_addr )
        : video( fb_addr ) {}

    Status putch( u8 c ) {
        if ( c == '\b' && x > 0 ) {
            --x;
            return Status::SUCCESS;
        }
        if ( c == '\r' || c == '\b' ) {
            x = 0;
            return Status::SUCCESS;
        }

        if ( c == '\n' || x >= columns )
            x = 0, y++;

        if ( y >= lines )
        {
            y = lines - 1;
            scroll();
        }

        if ( c == '\n' )
            return Status::SUCCESS;

        int idx = ( x + y * columns ) * 2;
        video[ idx ] = c;
        video[ idx + 1 ] = flags;

        ++ x;

        return Status::SUCCESS;
    }

    void clear()
    {
        for ( int i = 0; i < lines * columns * 2; ++i )
            video[ i ] = 0;
    }

    void scroll()
    {
        int i = 0;
        for ( ; i < ( lines - 1 ) * columns * 2; ++i )
            video[ i ] = video[ i + columns * 2 ];
        for ( ; i < lines * columns * 2; ++i )
            video[ i ] = 0;
    }
};

} /* dev */
} /* masys */

#endif /* end of include guard: _MASYS_DEV_VGA_H_ */
