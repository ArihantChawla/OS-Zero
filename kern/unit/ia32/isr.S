#define __ASSEMBLY__ 1
#include <kern/conf.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/macro.S>

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
 * CPU pushes EFLAGS, CS, and EIP, as well as an ERROR-code word if used.
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

//.extern	irqtimerfired

.globl	trapde, trapdb, trapnmi, trapbp, trapof, trapbr, trapud, trapnm, trapdf
.globl	trapts, trapnp, trapss, trapgp, trappf, trapmf, trapac, trapmc, trapxf
.globl	irqtmr0, irqtmr, irqkbd, irqmouse, irqerror, irqspurious
.globl	irqvec
#if (SMP) || (APIC)
.globl	irqerror, irqspurious, mpspurint, irqtmrcnt
#endif

.extern	sigfunc, tmrsyncscr, schedpickthr, vmpagefault, kbdtrap, mousetrap
.extern	thrsave, thrjmp
#if (APIC)
.extern	mpapic
#endif

.text	32

trapde:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPDE
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapdb:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPDB
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapnmi:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPNMI
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapbp:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPBP
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapof:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPOF
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapbr:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPBR
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapud:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPUD
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapnm:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPNM
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapdf:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPDF
	call	sigfunc

	movl	%ebp, %esp
	_traploadregserr
	sti
	iret

trapts:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPTS
	call	sigfunc

	movl	%ebp, %esp
	_traploadregserr
	sti
	iret
trapnp:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPNP
	call	sigfunc

	movl	%ebp, %esp
	_traploadregserr
	sti
	iret

trapss:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPSS
	call	sigfunc

	movl	%ebp, %esp
	_traploadregserr
	sti
	iret

trapgp:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPGP
	call	sigfunc

_gpfin:	
	movl	%ebp, %esp
	_traploadregserr
	sti
	iret

trappf:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	%eax			// error code
	movl	%cr2, %eax		// linear address
	pushl	%eax
	pushl	$0x00000000		// TODO: push PID here
	call	vmpagefault

	movl	%ebp, %esp
	_traploadregserr
	sti
	iret

trapmf:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPMF
	call	sigfunc

	movl	%ebp, %esp
	_traploadregserr
	sti
	iret

trapac:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPAC
	call	sigfunc

	movl	%ebp, %esp
	_traploadregserr
	sti
	iret

trapmc:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPMC
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

trapxf:
	_trapsaveregs
	movl	%esp, %ebp

	pushl	$TRAPXF
	call	sigfunc

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

#if (APIC)
	/* dummy interrupt handler for probing APIC frequency */
irqtmrcnt:
	_apiceoi
	iret
#endif

	/*
	 * simple interface to let the PIT do sleep() and such actions early in
	 * the bootstrap process
	 */
	/* dummy interrupt handler */
irqtmr0:
	/* push general purpose registers */
	pushal
	/* store stack pointer */
	movl	%esp, %ebp
	/* load interrupt vector address */
	movl	$irqvec, %eax
	/* load interrupt handler address */
	movl	0(%eax), %ebx
	/* call if non-zero */
	cmpl	$0, %ebx
	jz	_tmr0fin
	call	*%ebx
	/* zero interrupt handler address */
	movl	$irqvec, %eax
	movl	$0x00000000, 0(%eax)

_tmr0fin:
	_piteoi1
	movl	%ebp, %esp
	/* pop general purpose registers */
	popal
	/* re-enable interrupts */
	sti
	/* return */
	iret
	
irqtmr:
	/* store frame pointer */
	pushl	%ebp
	/* store %ecx */
	pushl	%ecx
	/* store stack pointer */
	movl	%esp, %ebp
	/* load [per-CPU] thread structure address */
	movl	%gs:8, %ecx
	leal	580(%ecx), %esp
	/* load %ebx original value */
	movl	(%ebp), %ecx
	/* push general purpose registers into thread control block */
	pushal
	/* restore frame pointer */
	leal	4(%ebp), %esp
	popl	%ebp
	/* arguments for thrsave */
	movl	(%esp), %edx		// return address
	movl	%gs:8, %eax		// thread structure address
	/* push 3rd argument (frame pointer) */
	pushl	%ebp
	/* push 2nd argument (return address) */
	pushl	%edx
	/* push 1st argument (thread structure address) */
	pushl	%eax
	call	thrsave
	/* schedpickthr() returns thread ID in eax */
	movl	schedpickthr, %ebx
	call	*%ebx
	/* thread ID argument in eax */
	call	thrjmp

irqkbd:
	_trapsaveregs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	4(%eax), %ebx
	jz	_irqkbddone
	call	*%ebx
_irqkbddone:	
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	
	movl	%ebp, %esp
	_traploadregs
	sti
	iret

irq3:
	_trapsaveregs
	movl	%esp, %ebp

#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	
	movl	%ebp, %esp
	_traploadregs
	sti
	iret

irq4:
	_trapsaveregs
	movl	%esp, %ebp

#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	
	movl	%ebp, %esp
	_traploadregs
	sti
	iret

irq5:
	_trapsaveregs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	20(%eax), %ebx
	jz	_irq5done
	call	*%ebx
_irq5done:	
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	
	movl	%ebp, %esp
	_traploadregs
	sti
	iret

irqmouse:
	_trapsaveregs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	48(%eax), %ebx
	jz	_irqmousedone
	call	*%ebx
_irqmousedone:	
#if (APIC)
	_apiceoi
#else
	_piteoi1
	_piteoi2
#endif
	
	movl	%ebp, %esp
	_traploadregs
	sti
	iret

#if (SMP) || (APIC)

irqerror:
	sti
	iret

irqspurious:
	_trapsaveregs
	movl	%esp, %ebp

	call	mpspurint

	movl	%ebp, %esp
	_traploadregs
	sti
	iret

#endif /* SMP || APIC */

.data

.align	NBPG

	/* interrupt vector */
	/* 0x00..0x1f	- IRQs 0-15 */
	/* 19		- IRQERROR */
	/* 29		- IRQIPI */
	/* 31		- IRQSPURIOUS */
irqvec:
	.space	(8 * NINTR)

