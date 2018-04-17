#ifndef _MASYS_SYSCALL_SYSCALL_HPP_
#define _MASYS_SYSCALL_SYSCALL_HPP_

namespace masys {

class InterruptManager;
void setup_syscalls();

/* SYSTEM CALLS
 * Calling convention:
 *      1. syscall number in %eax
 *      2. arguments on stack (first on top <==> pushed from right to left)
 *      3. raise software interrupt 0xAD
 *      4. return value is in %eax:%edx (%edx is 0 for int)
 * Register preservation: All registers (except for %eax and %edx, of course)
 *   are preserved.
 * Errors: Unless stated otherwise, negative value is negated errno.
 */
namespace syscall {

/* syscall 0
 * void cease( int code )
 * stops execution with exit code
 */
// (assembler magic that does not have prototype here)

/* syscall 1
 * int debug( int x )
 * prints number x and returns x
 */
int debug( int );

/* syscall 2
 * int prove()
 * returns version of the system
 */
int prove();

/* syscall 3
 * unsigned obtain( unsigned n, unsigned flags )
 * gets n new pages, return values < 4096 are errno codes, otherwise virtual
 * address of the newly allocated memory.
 * flags must be 0
 */
unsigned obtain( unsigned, unsigned );

/* syscall 4
 * TODO
 */

/* syscall 5
 * int inscribe( fd_t fd, const char *data, unsigned size, unsigned flags )
 * writes bytes from data to file descriptor fd
 * flags must be 0
 * returns number of bytes actually written
 */
int inscribe( unsigned, const char*, unsigned, unsigned );

} /* syscall */

} /* masys */

#endif /* end of include guard: _MASYS_SYSCALL_SYSCALL_HPP_ */
