#ifndef _MASYS_CONFIG_HPP_
#define _MASYS_CONFIG_HPP_

#define MASYS_VERSION           0x00000001
#define MASYS_PAGE_SIZE         0x1000
#define MASYS_HIGHER_HALF       0xC0000000
#define MASYS_VIRT_STACK        0xFFC00000
#define MASYS_VGA_MEMORY        0xC00B8000
#define MASYS_STACK_MAGIC       0x01D57ACC
#define MASYS_N_SYSCALLS        32
#define MASYS_N_INTERRUPTS      256
#define MASYS_SYSCALL_INTERRUPT 0xAD

#ifndef ASM_FILE
#ifdef __cplusplus
namespace masys {

const unsigned VERSION = MASYS_VERSION;
const unsigned PAGEDIR_ENTRIES = 1024;
const unsigned PAGE_SIZE = MASYS_PAGE_SIZE;
const unsigned HIGHER_HALF = MASYS_HIGHER_HALF;

namespace mem::reserved {
const unsigned USER_HEAP_BEGIN = 0x1000;
const unsigned USER_HEAP_END = HIGHER_HALF;
const unsigned FRAME_ALLOC_BITMAP_START = HIGHER_HALF + 0x1000;
const unsigned HEAP_BEGIN = HIGHER_HALF + 0x400000;
const unsigned STACK_BOTTOM = MASYS_VIRT_STACK;
const unsigned HEAP_END = STACK_BOTTOM - 0x400000;
const unsigned PAGE_DIRECTORY = STACK_BOTTOM;
} /* reserved */


} /* masys */
#endif
#endif

#endif /* end of include guard: _MASYS_CONFIG_HPP_ */
