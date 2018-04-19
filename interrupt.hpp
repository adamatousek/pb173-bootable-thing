#ifndef _MASYS_INTERRUPT_HPP_
#define _MASYS_INTERRUPT_HPP_

#include "types.hpp"
#include "config.hpp"

namespace masys {

struct IdtEntry {
    u16 offset_low;
    u16 selector;
    u8  _unused;
    union {
        struct {
            u8 type : 4;
            u8 storage_segment : 1;
            u8 dpl : 2;
            u8 present : 1;
        };
        u8 _type_attr;
    };
    u16 offset_high;

    IdtEntry() : _unused( 0 ), _type_attr( 0 ) {}
    u32 offset() const
    {
        return u32( offset_high ) << 16 | offset_low;
    }
    void offset( u32 off )
    {
        offset_low = off;
        offset_high = off >> 16;
    }
    template< typename T >
    void offset( T *ptr ) { offset( reinterpret_cast< u32 >( ptr ) ); }
};

static_assert( sizeof( IdtEntry ) == 8, "IDT Entry has wrong size." );

class InterruptManager {
public:
    static constexpr u32 N_INTERRUPTS = MASYS_N_INTERRUPTS;
    static constexpr u32 N_SYSCALLS = MASYS_N_SYSCALLS;
    static constexpr u8 SYSCALL_INTERRUPT = MASYS_SYSCALL_INTERRUPT;

private:
    IdtEntry idt[ 265 ];
    void register_syscall( unsigned char, void *, unsigned char );

public:
    InterruptManager();

    void enable( bool e ) {
        if ( e )
            sti();
        else
            cli();
    }

    template< typename Ret, typename ... Args >
    void register_syscall( unsigned char num, Ret (*fn)(Args...) )
    {
        constexpr unsigned char argc = sizeof...( Args );
        register_syscall( num, reinterpret_cast< void * >( fn ), argc );
    }

    void cli();
    void sti();

    static void dummy_handler( unsigned, unsigned, unsigned );
    static void deadly_handler( unsigned, unsigned, unsigned );
};

extern InterruptManager * intr;

static const char *INTR_NAME[] = {
    /*  0 */ "Division by zero",
    /*  1 */ "Reserved 0x01",
    /*  2 */ "NMI",
    /*  3 */ "Breakpoint",
    /*  4 */ "Overflow",
    /*  5 */ "Out of range",
    /*  6 */ "Invalid opcode - UD2",
    /*  7 */ "Device not available",
    /*  8 */ "Double fault",
    /*  9 */ "Coprocessor segment overrun",
    /* 10 */ "Invalid TSS",
    /* 11 */ "Segment not present",
    /* 12 */ "Stack-segment fault",
    /* 13 */ "General protection fault",
    /* 14 */ "Page fault",
    /* 15 */ "Reserved 0x0F",
    /* 16 */ "FPU error",
    /* 17 */ "Alignment check",
    /* 18 */ "Machine check",
    /* 19 */ "SIMD float exception",
    /* 20 */ "Reserved 0x14",
    /* 21 */ "Reserved 0x15",
    /* 22 */ "Reserved 0x16",
    /* 23 */ "Reserved 0x17",
    /* 24 */ "Reserved 0x18",
    /* 25 */ "Reserved 0x19",
    /* 26 */ "Reserved 0x1A",
    /* 27 */ "Reserved 0x1B",
    /* 28 */ "Reserved 0x1C",
    /* 29 */ "Reserved 0x1D",
    /* 30 */ "Reserved 0x1E",
    /* 31 */ "Reserved 0x1F",
};

} /* masys */

#endif /* end of include guard: _MASYS_INTERRUPT_HPP_ */
