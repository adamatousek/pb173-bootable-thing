#include "_masys_syscall.h"

int prove()
{
    HANDLE_ERRNO_SYSCALL( syscall( 2 ) );
}

