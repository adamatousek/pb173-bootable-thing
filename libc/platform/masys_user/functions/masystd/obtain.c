#include "_masys_syscall.h"

void * obtain( unsigned pages, unsigned flags )
{
    int m = syscall( 3, pages, flags );
    if ( m < 4096 ) {
        errno = m;
        return 0;
    }
    errno = 0;
    return (void *) m;
}

