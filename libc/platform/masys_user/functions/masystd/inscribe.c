#include "_masys_syscall.h"

int inscribe( fd_t fd, const char *data, unsigned sz, unsigned flags )
{
    HANDLE_ERRNO_SYSCALL( syscall( 5, fd, data, sz, flags ) );
}

