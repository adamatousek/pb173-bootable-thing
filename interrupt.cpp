#include "interrupt.hpp"
#include "debug.hpp"
#include "panic.hpp"

namespace masys {

using InterruptHandler = void(*)(unsigned, unsigned, unsigned);
using SyscallHandler = void *;

extern "C" {

// Defined in interrupt_asm.S
void __masys_setup_idt( u32 sz, void *idt );
void __masys_cli();
void __masys_sti();
unsigned __masys_cr2();

// Defined in syscall/syscall_asm.S
void __masys_syscall_handler();

extern char __masys_intr_handler[ 8 * InterruptManager::N_INTERRUPTS ];
extern InterruptHandler __masys_interrupt_handlers[ InterruptManager::N_INTERRUPTS ];
extern SyscallHandler __masys_syscall_fn_table[ InterruptManager::N_SYSCALLS ];
extern u8 __masys_syscall_argc_table[ InterruptManager::N_SYSCALLS ];
}

InterruptManager::InterruptManager()
{
    cli();
    __masys_setup_idt( N_INTERRUPTS * sizeof( IdtEntry ) - 1, idt );
    for( int i = 0; i < N_INTERRUPTS; ++i ) {
        if ( i == 0 || i == 5 || i == 13 || i == 14 )
            __masys_interrupt_handlers[ i ] = deadly_handler;
        else
            __masys_interrupt_handlers[ i ] = dummy_handler;
        idt[ i ].type = 0xe; // Interrupt gate
        idt[ i ].offset( __masys_intr_handler + 8 * i );
        idt[ i ].selector = 0x08; // Kernel text
        idt[ i ].present = true;
    }

    // Syscall
    idt[ SYSCALL_INTERRUPT ].dpl = 3;
    idt[ SYSCALL_INTERRUPT ].offset( __masys_syscall_handler );
    sti();
}

void InterruptManager::register_syscall( u8 num, void *call, u8 argc )
{
    if ( num >= N_SYSCALLS )
        return;

    __masys_syscall_argc_table[ num ] = argc;
    // TODO: memory barrier here
    __masys_syscall_fn_table[ num ] = call;
}

void InterruptManager::cli()
{
    __masys_cli();
}

void InterruptManager::sti()
{
    __masys_sti();
}

void InterruptManager::dummy_handler( unsigned intn, unsigned errc, unsigned )
{
    dbg::sout() << "Got interrupt #" << intn;
    if( intn < 32 )
        dbg::sout() << " (" << INTR_NAME[ intn ] << ')';
    dbg::sout() << '\n';
    dbg::sout() << "Error code: 0x" << dbg::hex() << errc << '\n';
}

void InterruptManager::deadly_handler( unsigned intn, unsigned errc, unsigned addr )
{
    dbg::vout() << "!!!\nGot lethal interrupt #" << intn;
    if( intn < 32 )
        dbg::vout() << " (" << INTR_NAME[ intn ] << ')';
    dbg::vout() << "\n!!!";
    dbg::vout() << "Error code: 0x" << dbg::hex() << errc << '\n';

    dbg::sout() << "\n!!! Got lethal interrupt #" << intn;
    if( intn < 32 )
        dbg::sout() << " (" << INTR_NAME[ intn ] << ')';
    dbg::sout() << " !!!\n";
    dbg::sout() << "Error code: 0x" << dbg::hex() << errc << '\n';
    if ( intn == 14 ) {
        dbg::sout() << "Faulty address: 0x" << dbg::hex() << __masys_cr2()
                    << "\nViolator: 0x" << addr << '\n';
    }

    panic();
}

} /* masys */
