#include <mach/arm/setup.h>
#include <mach/arm/int.h>

.globl _start, _intinit

.extern kloop

.text 32

.org 0x00000000

/*
 * interrupt vector table with branch instructions to handler stubs
 * - also entry point at 0x00000000
 */
_start:
_intvec:
	b	_intreset
	b	_intundef
	b	_intswi
	b	_intpref
	b	_intdata
	b	_intadr		// not assigned / emulation
	b	_intirq
/* as last entry, fast interrupt requests get serviced in-place */
_intfiq:
	subs	lr, #4			// adjust link register (return address)
	/* handler code/call here */
	_FIQUNMASK		// unmask FIQs
	_IRQUNMASK		// unmask FIQs
	bx	lr
	
_intundef:
	/* code here */
	_IRQUNMASK
	bx	lr

_intswi:
	/* code here */
	_IRQUNMASK
	bx	lr

_intpref:
	subs	lr, #4
	/* code here */
	_IRQUNMASK
	bx	lr

_intdata:
	subs	lr, #8
	/* code here */
	_IRQUNMASK
	bx	lr

_intadr:
	/* code here */

_intirq:
	subs	lr, #4
	/* handler call here */
	_IRQUNMASK
	bx	lr

_intreset:
	/* initialise interrupt stack */
	mov	r1, #INTDISAB|INTIRQ32
	msr	cpsr_c, r1
	mov	sp, #_intstk
	/* svc stack */
	mov	r1, #INTDISAB|INTSVC32
	msr	cpsr_c, r1
	mov	sp, #_svcstk
	/* user stack */
	mov	r1, #INTUSR32
	msr	cpsr_c, r1
	mov	sp, #_usrstk
	/* enable interrupts */
	_FIQUNMASK
	_IRQUNMASK
	b	kmain

_intstk:	
	.long	M_INTSTK
_svcstk:
	.long	M_SVCSTK
_usrstk:
	.long	M_USRSTK

