#include "syscall/syscall.hpp"
#include "mem/vmmap.hpp"
#include <errno.h>

namespace masys {
namespace syscall {

unsigned obtain( unsigned npages, unsigned flags )
{
    if ( flags )
        return EINVAL;
    if ( npages > 786400 )
        return ENOMEM; /* ENASRAT, noone needs so much mem */;

    unsigned m = mem::page_allocator->alloc( npages, /* user = */ true );

    if ( m == 0 )
        return ENOMEM;

    return m;
}

} /* syscall */
} /* masys */
