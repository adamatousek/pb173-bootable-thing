#ifndef _MASYS_DEV_OUTPUT_HPP_
#define _MASYS_DEV_OUTPUT_HPP_

#include <types.hpp>

#include <dev/device.hpp>
#include <debug.hpp>

namespace masys {
namespace dev {

class CharacterOutput : public Device {
public:
    virtual Status putch( u8 ) = 0;
    virtual int puts( const char * asciiz ) {
        int written = 0;
        while ( *asciiz && putch( *asciiz++ ) == Status::SUCCESS )
            ++written;
        return written;
    }
    virtual int write( const char * data, unsigned size ) {
        unsigned written = 0;
        while ( written++ < size && putch( *data++ ) == Status::SUCCESS )
            ;
        return written - 1;
    }
};

} /* dev */
} /* masys */

#endif /* end of include guard: _MASYS_DEV_OUTPUT_HPP_ */
