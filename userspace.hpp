#ifndef _MASYS_USERSPACE_HPP_
#define _MASYS_USERSPACE_HPP_

namespace masys {

int userjmp( unsigned text, unsigned stack, unsigned *tss_esp0 );

// Dummy userspace function
void hello_kernel();

} /* masys */

extern "C" {

int syscall( unsigned char sysnum, ... );

}

#endif /* end of include guard: _MASYS_USERSPACE_HPP_ */
