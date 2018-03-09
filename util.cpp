#include <util.hpp>

namespace masys {

u32 itoa( u32 n, char * dst, u8 base )
{
    u32 l = 0;

    if ( n == 0 )
    {
        dst[0] = '0';
        ++l;
    }

    while ( n > 0 )
    {
        u8 d = n % base;
        for ( int i = l; i > 0; --i )
            dst[ i ] = dst[ i - 1 ];
        dst[ 0 ] = d + ( d > 9 ? 'a' - 10 : '0');
        n /= base;
        ++l;
    }

    dst[ l ] = 0;
    return l;
}

} /* masys */
