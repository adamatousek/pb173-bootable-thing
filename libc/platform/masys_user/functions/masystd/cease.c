#include "_masys_syscall.h"

void cease( int exitnum )
{
    syscall( 0, exitnum );
}

