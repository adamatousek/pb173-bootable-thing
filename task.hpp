#ifndef _MASYS_TASK_HPP_
#define _MASYS_TASK_HPP_

namespace masys {

struct Registers {
    long edi, // PUSHA compatible
         esi,
         ebp,
         esp0,
         ebx,
         edx,
         ecx,
         eax;
    long eip, // IRET compatible
         cs,
         flags,
         esp,
         ss;
};

struct Task {
    enum SchedState {
        READY,
        RUNNING,
        FOCUSED,
        BLOCKED,
        DEAD,
    } sched_state;
    unsigned short pid;
    unsigned pgdir_phys;
    Registers regs;
};

} /* masys */

#endif /* end of include guard: _MASYS_TASK_HPP_ */
