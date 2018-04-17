#if 1
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
#include <masystd.h>

void * _PDCLIB_allocpages( size_t n )
{
    return obtain( n, 0 );
}

#endif
