#include "syscall/syscall.hpp"
#include "interrupt.hpp"
#include "config.hpp"

namespace masys {

extern "C" {
int __masys_sys_cease( unsigned );
}

void setup_syscalls()
{
    intr->register_syscall( 0, __masys_sys_cease );
    intr->register_syscall( 1, syscall::debug );
    intr->register_syscall( 2, syscall::prove );
    intr->register_syscall( 3, syscall::obtain );
}

namespace syscall {

int prove()
{
    return VERSION;
}

} /* syscall */

} /* masys */
