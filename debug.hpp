#ifndef _MASYS_DEBUG_HPP_
#define _MASYS_DEBUG_HPP_

#include <types.hpp>

namespace masys {
namespace dev {
class Vga;
class SerialLine;
class CharacterOutput;
}

namespace dbg {

extern dev::Vga *vga;
extern dev::SerialLine *ser;


struct BaseMod {
    u8 base;
    BaseMod( u8 base ) : base( base ) {}
};

inline BaseMod hex() { return BaseMod( 16 ); }
inline BaseMod dec() { return BaseMod( 10 ); }
inline BaseMod oct() { return BaseMod( 8 ); }
inline BaseMod bin() { return BaseMod( 2 ); }

class OutStream {
    dev::CharacterOutput *out;
    u8 base = 10;

public:
    OutStream( dev::CharacterOutput *o ) : out( o ) {}
    OutStream & operator<<( const char * str );
    OutStream & operator<<( char c );
    OutStream & operator<<( unsigned n );
    OutStream & operator<<( void *p ) {
        return *this << "0x" << hex() << unsigned( p ) << BaseMod( base );
    }
    OutStream & operator<<( unsigned short n ) { return *this << unsigned( n ); }
    OutStream & operator<<( short n ) { return *this << int( n ); }
    OutStream & operator<<( int n ) { return *this << unsigned( n ); }
    OutStream & operator<<( long n ) { return *this << int( n ); }
    OutStream & operator<<( unsigned long n ) { return *this << unsigned( n ); }
    OutStream & operator<<( BaseMod b ) {
        base = b.base;
        return *this;
    }
};

OutStream vout();
OutStream sout();

void hlt();

} /* dbg */
} /* masys */

#endif /* end of include guard: _MASYS_DEBUG_HPP_ */
