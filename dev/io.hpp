#ifndef _MASYS_DEV_IO_HPP_
#define _MASYS_DEV_IO_HPP_

extern "C" {
// Defined in dev/io.S
// All the longs are actually supposed to be chars
void __masys_outb( unsigned long port, unsigned long data );
unsigned __masys_inb( unsigned long port );
}

namespace masys {
namespace dev {
    void outb( u32 port, u8 data ) { __masys_outb( port, data ); }
    u8 inb( u32 port ) { return __masys_inb( port ); }
} /* dev */
} /* masys */

#endif /* end of include guard: _MASYS_DEV_IO_HPP_ */
