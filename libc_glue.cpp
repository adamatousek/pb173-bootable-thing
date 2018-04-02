#include <dev/output.hpp>
#include <dev/input.hpp>
#include <mem/vmmap.hpp>
#include <debug.hpp>
#include <panic.hpp>

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "_PDCLIB_glue.h"

namespace {
masys::mem::PageAllocator *pal;
}

extern "C" {

void * _PDCLIB_allocpages( size_t n )
{
    return reinterpret_cast< void * >( pal->alloc( n ) );
}

void _PDCLIB_freepages( void * p, size_t n )
{
    pal->free( reinterpret_cast< unsigned long >( p ), n );
}

void _PDCLIB_Exit( int )
{
    panic();
}

static bool readf( _PDCLIB_fd_t self, void * buff, size_t length,
                   size_t * numBytesRead )
{
    auto indev = reinterpret_cast< masys::dev::CharacterInput * >( self.pointer );
    auto buf = reinterpret_cast< char * >( buff );
    int read = 0;
    unsigned char c;
    while ( read < length && indev->getch( c ) == masys::Status::SUCCESS ) {
        ++read;
        *buf++ = c;
    }
    *numBytesRead = read;
    return true;
}

static bool writef( _PDCLIB_fd_t self, const void * buff, size_t length, 
                   size_t * numBytesWritten )
{
    auto outdev = reinterpret_cast< masys::dev::CharacterOutput * >( self.pointer );
    auto buf = reinterpret_cast< const char * >( buff );
    int written = 0;
    while ( written < length && outdev->putch( *buf++ ) == masys::Status::SUCCESS )
        ++written;
    *numBytesWritten = written;
    return true;
}
static bool seekf( _PDCLIB_fd_t self, int_fast64_t offset, int whence,
    int_fast64_t* newPos )
{
    return false;
}

static void closef( _PDCLIB_fd_t self )
{
}

extern _PDCLIB_fileops_t _PDCLIB_fileops;
extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;

}

namespace masys {

void init_glue( mem::PageAllocator *pa, dev::SerialLine *se, dev::Vga *vga )
{
    pal = pa;

    _PDCLIB_fileops.read  = readf;
    _PDCLIB_fileops.write = writef;
    _PDCLIB_fileops.seek  = seekf;
    _PDCLIB_fileops.close = closef;

    stdin->handle.pointer = se;
    stdout->handle.pointer = se;
    vgaout->handle.pointer = vga;
    stderr->handle.pointer = se;
}

}
