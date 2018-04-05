#ifndef _MASYS_USERSPACE_HPP_
#define _MASYS_USERSPACE_HPP_

namespace masys {

void userjmp( unsigned text, unsigned stack, unsigned *tss_esp0 );

// Dummy userspace function
void hello_kernel();

} /* masys */

#endif /* end of include guard: _MASYS_USERSPACE_HPP_ */
