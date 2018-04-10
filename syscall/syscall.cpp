#include "syscall/syscall.hpp"
#include "interrupt.hpp"
#include "debug.hpp"

namespace masys {

extern "C" {
int __masys_sys_cease( unsigned );
}

void setup_syscalls()
{
    intr->register_syscall( 0, __masys_sys_cease );
    intr->register_syscall( 1, syscall::debug );
}

} /* masys */
