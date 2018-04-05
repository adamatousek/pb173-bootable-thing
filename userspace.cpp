#include "userspace.hpp"

namespace masys {

extern "C" {
// Defined in interrupt_asm.S
void __masys_userjmp( unsigned, unsigned, unsigned * );
}

void userjmp( unsigned text, unsigned stack, unsigned *tss_esp0 )
{
    __masys_userjmp( text, stack, tss_esp0 );
}

void hello_kernel()
{
    // check stack pointer in gdb
    asm( "int   $0xAD" ); // syscall
    asm( "int   $0xDA" ); // userleave
}

} /* masys */
