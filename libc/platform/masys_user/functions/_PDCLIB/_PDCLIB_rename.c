#if 1
/* _PDCLIB_rename( const char *, const char * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <stdio.h>

#ifndef REGTEST
#include "_PDCLIB_glue.h"
#include <errno.h>

int _PDCLIB_rename( const char * old, const char * new )
{
    errno = ENOTSUP;
    return -1;
}

#endif

#endif
