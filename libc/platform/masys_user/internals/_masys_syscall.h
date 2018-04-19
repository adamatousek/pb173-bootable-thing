#include "masystd.h"
#include <errno.h>

#define HANDLE_ERRNO_SYSCALL( call ) do {\
        long retval = call;\
        if( retval < 0 ){\
            errno = - retval;\
            return -1;\
        }\
        errno = 0;\
        return retval;\
    } while( 0 );

