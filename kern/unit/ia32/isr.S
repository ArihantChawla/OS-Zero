#define __ASSEMBLY__ 1
#include <kern/conf.h>
#include <kern/unit/x86/trap.h>

/*
 * NOTE: we use interrupt gates, which automatically disable further
 * interrupts.
 */

/*
 * FIXME
 * - make the interrupt handlers add to interrupt mask, not set it
 * - use local APIC instead of PIC + PIT
 */

/*
 * CPU pushes EFLAGS, CS, and EIP and an ERROR-code word if used.
 * return with IRET after return from handler.
 *
 * top of stack after interrupt
 * 
 * eflags
 * cs
 * eip		<- ESP if error code not present
 * (err)	<- ESP if error code supplied
 */

/*
 * interrupt prologue
 * ------------------
 * ESP points to IRET stack frame OR ERROR-code right below it.
 * push general-purpose registers with PUSHA.
 * possible ERROR-code is passed in EAX.
 */

/*
 * interrupt epilogue
 * ------------------
 * interrupt routines with error must adjust stack pointer.
 * in the end, we enable interrupts with STI.
 */

.globl	trapde, trapdb, trapnmi, trapbp, trapof, trapbr, trapud, trapnm, trapdf
.globl	trapts, trapnp, trapss, trapgp, trappf, trapmf, trapac, trapmc, trapxf
.globl	irqtimer0, irqtimer, irqkbd, irqmouse, irqerror, irqspurious
.globl	irqvec
#if (SMP) || (APIC)
.globl	irqerror, irqspurious, mpspurint
#endif

.extern	sigfunc, tmrsyncscr, schedyield, vmpagefault, kbdtrap, mousetrap
.extern	thrsave, thrjmp

.text	32

trapde:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPDE
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapdb:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPDB
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapnmi:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPNMI
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapbp:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPBP
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapof:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPOF
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapbr:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPBR
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapud:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPUD
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapnm:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPNM
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapdf:
	cli
	pusha
	popl	%eax
//	movl	32(%esp), %ebx
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPDF
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	addl	$4, %esp
	popal
	sti
	iret

trapts:
	cli
//	movl	32(%esp), %eax
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPTS
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret
trapnp:
	cli
//	movl	32(%esp), %eax
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPNP
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapss:
	cli
//	movl	32(%esp), %eax
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPSS
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapgp:
	cli
	/* EAX	- fault IP */
//	movl	32(%esp), %eax
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPGP
	call	sigfunc

_gpfin:	
	cli
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trappf:
	cli
//	movl	32(%esp), %eax
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	%eax			// error code
	movl	%cr2, %eax		// linear address
	pushl	%eax
	pushl	$0x00000000		// TODO: push PID here
	call	vmpagefault

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapmf:
	cli
	/* no error on stack */
//	movl	32(%esp), %eax
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPMF
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapac:
	cli
	/* fault eip <- stack pointer */
//	movl	32(%esp), %eax
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPAC
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapmc:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPMC
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapxf:
	cli
	/* fault eip <- stack pointer */
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPXF
	call	sigfunc

	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irqtimer0:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	jz	_tmr0fin
	movl	0(%eax), %ebx
	jz	_tmr0fin
	call	*%ebx
	movl	$irqvec, %eax
	movl	$0x00000000, 0(%eax)

_tmr0fin:
	movb	$0x20, %al
	outb	%al, $0x20
	
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	movl	$1, irqtimerfired
	sti
	iret
	
irqtimer:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	/* arguments for thrsave */
	movl	48(%ebp), %edx		// return address
	movl	%gs:8, %eax
	call	thrsave
	/* schedyield returns thread ID in eax */
	movl	schedyield, %ebx
	call	*%ebx
	movl	%eax, %ebx
	movl	%ebx, %eax
	call	thrjmp

irqkbd:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	4(%eax), %ebx
	jz	_irqkbddone
	call	*%ebx
_irqkbddone:	
	movb	$0x20, %al
	outb	%al, $0x20

	movb	$0x20, %al
	outb	%al, $0x20
	
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irq3:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	movb	$0x20, %al
	outb	%al, $0x20
	
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irq4:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	movb	$0x20, %al
	outb	%al, $0x20
	
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irq5:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	20(%eax), %ebx
	jz	_irq5done
	call	*%ebx
_irq5done:	
	movb	$0x20, %al
	outb	%al, $0x20
	
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irqmouse:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	48(%eax), %ebx
	jz	_irqmousedone
	call	*%ebx
_irqmousedone:	
	movb	$0x20, %al
	outb	%al, $0xa0
	outb	%al, $0x20
	
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

#if (SMP) || (APIC)
irqerror:
	sti
	iret

irqspurious:
	cli
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	call	mpspurint

	movb	$0x20, %al
	outb	%al, $0x20
	
	movl	%ebp, %esp
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret
#endif

.data

.align	32

	/* 0x00..0x1f	- IRQs 0-15 */
	/* 19		- IRQERROR */
	/* 29		- IRQIPI */
	/* 31		- IRQSPURIOUS */
irqvec:
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
#if (SMP) || (APIC)
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
	.long	0x00000000
#endif

