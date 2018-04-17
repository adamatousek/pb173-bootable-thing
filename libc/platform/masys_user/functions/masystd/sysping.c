#include "_masys_syscall.h"

int sysping( int test )
{
    HANDLE_ERRNO_SYSCALL( syscall( 1, test ) );
}

