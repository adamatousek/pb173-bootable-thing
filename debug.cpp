#include <debug.hpp>
#include <dev/vga.hpp>
#include <dev/serial.hpp>
#include <util.hpp>

extern "C" {
void __masys_hlt();
}

namespace masys {
namespace dbg {

OutStream & OutStream::operator<<( char c ) {
    out->putch( c );
    return *this;
}

OutStream & OutStream::operator<<( const char * str ) {
    out->puts( str );
    return *this;
}

OutStream & OutStream::operator<<( unsigned n ) {
    char buf[ 32 ];
    itoa( n, buf, base );
    out->puts( buf );
    return *this;
}

OutStream vout() { return OutStream( vga ); }
OutStream sout() { return OutStream( ser ); }

void hlt()
{
    __masys_hlt();
}

} /* dbg */

namespace syscall {
    int debug( int n )
    {
        dbg::sout() << "From syscall::debug: 0x" << dbg::hex() << n << "\n";
        return n;
    }
} /* syscall */

} /* masys */
