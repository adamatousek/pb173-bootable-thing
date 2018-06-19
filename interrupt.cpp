#include "interrupt.hpp"
#include "debug.hpp"
#include "panic.hpp"
#include "dev/io.hpp"

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

using dev::inb;
using dev::outb;

InterruptManager *InterruptManager::self = nullptr;
static constexpr unsigned char PIC_PORT_CMD[2] = { 0x20, 0xA0 };
static constexpr unsigned char PIC_PORT_DATA[2] = { 0x21, 0xA1 };


InterruptManager::InterruptManager()
{
    cli();
    self = this;
    // Interrupt handlers
    __masys_setup_idt( N_INTERRUPTS * sizeof( IdtEntry ) - 1, idt );
    for ( int i = 0; i < N_INTERRUPTS; ++i ) {
        if ( i == 0 || i == 5 || i == 13 || i == 14 )
            __masys_interrupt_handlers[ i ] = deadly_handler;
        else if ( i >= 0x20 && i < 0x30 )
            __masys_interrupt_handlers[ i ] = irq_switch_handler;
        else
            __masys_interrupt_handlers[ i ] = dummy_handler;
        idt[ i ].type = 0xe; // Interrupt gate
        idt[ i ].offset( __masys_intr_handler + 8 * i );
        idt[ i ].selector = 0x08; // Kernel text
        idt[ i ].present = true;
    }
    for ( int i = 0; i < N_IRQS; ++i ) {
        irq_handlers[ i ] = irq_dummy_handler;
    }

    // PIC setup
    pic_remap( PIC_MASTER, 0x20 );
    pic_remap( PIC_SLAVE, 0x28 );

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

void InterruptManager::pic_remap( bool pic, u8 base )
{
    outb( PIC_PORT_CMD[ pic ], 0x11 );
    outb( PIC_PORT_DATA[ pic ], base );

    if ( pic == PIC_MASTER )
        outb( PIC_PORT_DATA[ pic ], 0x04 );
    else
        outb( PIC_PORT_DATA[ pic ], 0x02 );

    outb( PIC_PORT_DATA[ pic ], 0x01 );
    irq_base[ pic ] = base;
}

void InterruptManager::pic_reenable( u8 irq )
{
    if ( irq >= 8 )
        outb( PIC_PORT_CMD[ PIC_SLAVE ], 0x20 );
    outb( PIC_PORT_CMD[ PIC_MASTER ], 0x20 );
}

void InterruptManager::pic_disable( u8 irq )
{
    bool pic = irq >= 8;
    irq %= 8;
    u8 mask = inb( PIC_PORT_DATA[ pic ] );
    mask |= ( 1 << irq );
    outb( PIC_PORT_DATA[ pic ], mask );
}

void InterruptManager::pic_enable( u8 irq )
{
    bool pic = irq >= 8;
    irq %= 8;
    u8 mask = inb( PIC_PORT_DATA[ pic ] );
    mask &= ~( 1 << irq );
    outb( PIC_PORT_DATA[ pic ], mask );
}

char InterruptManager::intr2irq( u8 intr )
{
    if ( intr >= irq_base[ PIC_MASTER ] && intr < irq_base[ PIC_MASTER ] + 8 )
        return intr - irq_base[ PIC_MASTER ];
    if ( intr >= irq_base[ PIC_SLAVE ] && intr < irq_base[ PIC_SLAVE ] + 8 )
        return intr - irq_base[ PIC_SLAVE ] + 8;
    return -1;
}

void InterruptManager::irq_switch_handler( unsigned intn, unsigned, unsigned )
{
    auto irq = self->intr2irq( intn );
    self->irq_handlers[ irq ]( irq );
    pic_reenable( irq );
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

void InterruptManager::irq_dummy_handler( unsigned char irqn )
{
    dbg::sout() << "Got IRQ " << +irqn << ".\n";
    pic_disable( irqn );
}

} /* masys */
