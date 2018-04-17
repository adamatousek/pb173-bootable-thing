#include "syscall.hpp"
#include "debug.hpp"
#include "config.hpp"
#include "dev/vga.hpp"
#include "dev/serial.hpp"
#include <errno.h>

namespace masys {
namespace syscall {

int inscribe( unsigned fd, const char* data, unsigned sz, unsigned flags )
{
    if ( flags )
        return -EINVAL;
    if ( fd > 1 )
        return -EBADF;
    unsigned d = reinterpret_cast< unsigned >( data );
    if ( d >= HIGHER_HALF || d + sz > HIGHER_HALF || data > data + sz )
        return -EINVAL; // Probably just #GP or something like that

    if ( fd == 0 )
        return dbg::ser->write( data, sz );
    if ( fd == 1 )
        return dbg::vga->write( data, sz );
}

} /* syscall */
} /* masys */
