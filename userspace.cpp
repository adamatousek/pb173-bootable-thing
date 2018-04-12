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
        syscall( 1, 0x666 );
#if 1
    int * a = reinterpret_cast< int * >( syscall( 3 /* syscall::obtain */, 1, 0 ) );
    if ( a < reinterpret_cast< int * >( 4096 ) )
        syscall( 0, 0x667 );
    *a = 42;
    syscall( 1, a );
    syscall( 1, *a );
#endif
    syscall( 1, syscall( 2 /* syscall::prove */ ) );
    syscall( 0 /* syscall::cease */, &x );
}

} /* masys */
