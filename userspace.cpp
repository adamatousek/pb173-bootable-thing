#include "userspace.hpp"

namespace masys {

extern "C" {
// Defined in interrupt_asm.S
int __masys_userjmp( unsigned, unsigned, unsigned * );
}

int userjmp( unsigned text, unsigned stack, unsigned *tss_esp0 )
{
    return __masys_userjmp( text, stack, tss_esp0 );
}

void hello_kernel()
{
    int x = 3;
    if ( syscall( 1 /* syscall::debug */, x ) != x )
        syscall( 1, -666 );
    syscall( 0 /* syscall::cease */, &x );
}

} /* masys */
