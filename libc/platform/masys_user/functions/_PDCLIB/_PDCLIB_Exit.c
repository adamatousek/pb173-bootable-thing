#if 1
/* _PDCLIB_Exit( int )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

/* This is a stub implementation of _PDCLIB_Exit
*/

#include <stdlib.h>

#ifndef REGTEST
#include "_PDCLIB_glue.h"
#include <masystd.h>

void _PDCLIB_Exit( int status )
{
    cease( status );
}

#endif
#endif
