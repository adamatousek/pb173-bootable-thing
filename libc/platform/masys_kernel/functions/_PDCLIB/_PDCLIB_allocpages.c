#if 0
/* _PDCLIB_allocpages( int const )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

/* This is a stub implementation of _PDCLIB_allocpages
*/

#include <stdint.h>
#include <stddef.h>
#include "_PDCLIB_glue.h"
#include <errno.h>

void * _PDCLIB_allocpages( size_t n )
{
    errno = ENOTSUP;
    return NULL;
}

#endif
