/* _PDCLIB_fileops

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include "_PDCLIB_glue.h"

#if 1
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <masystd.h>

static bool readf( _PDCLIB_fd_t self, void * buf, size_t length, 
                   size_t * numBytesRead )
{
    errno = ENOTSUP;
    return false;
}

static bool writef( _PDCLIB_fd_t self, const void * buf, size_t length, 
                   size_t * numBytesWritten )
{
    int w = inscribe( self.sval, buf, length, 0 );
    if ( w >= 0 ) {
        *numBytesWritten = w;
        return true;
    }
    *numBytesWritten = 0;
    return false;
}
static bool seekf( _PDCLIB_fd_t self, int_fast64_t offset, int whence,
    int_fast64_t* newPos )
{
    errno = ENOTSUP;
    return false;
}

static void closef( _PDCLIB_fd_t self )
{
    errno = ENOTSUP;
}

const _PDCLIB_fileops_t _PDCLIB_fileops = {
    .read  = readf,
    .write = writef,
    .seek  = seekf,
    .close = closef,
};

#endif
