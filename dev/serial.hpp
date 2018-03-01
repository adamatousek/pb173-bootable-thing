#ifndef _MASYS_DEV_SERIAL_HPP_
#define _MASYS_DEV_SERIAL_HPP_

#include <dev/input.hpp>
#include <dev/output.hpp>
#include <dev/io.hpp>

namespace masys {
namespace dev {

const u32 SERIAL_PORT_1 = 0x3f8;

class SerialLine : public CharacterOutput, public CharacterInput {
    u32 port;
public:
    SerialLine( u32 port ) : port( port ) {}

    Status reinit() override {
        outb( port + 1, 0x00 ); // Disable all interrupts
        outb( port + 3, 0x80 ); // Enable DLAB (set baud rate divisor)
        outb( port + 0, 0x03 ); // Set divisor to 3 (lo byte) 38400 baud
        outb( port + 1, 0x00 ); //                  (hi byte)
        outb( port + 3, 0x03 ); // 8 bits, no parity, one stop bit
        outb( port + 2, 0xC7 ); // Enable FIFO, clear them, with 14-byte threshold
        outb( port + 4, 0x0B ); // IRQs enabled, RTS/DSR set

        return Status::SUCCESS;
    }

    Status putch( u8 data ) override {
        while ( ( inb( port + 5 ) & 0x20 ) == 0 )
            ;
        outb( port, data );
        return Status::SUCCESS;
    }

    Status getch( u8 & data ) override {
        while ( ( inb( port + 5 ) & 0x01 ) == 0 )
            ;
        data = inb( port );
        return Status::SUCCESS;
    }
};

} /* dev */
} /* masys */

#endif /* end of include guard: _MASYS_DEV_SERIAL_HPP_ */
