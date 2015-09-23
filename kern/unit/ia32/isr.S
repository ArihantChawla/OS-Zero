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
 * possible ERROR-code is stored EDX.
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

.extern	sigfunc, tmrsyncscr, taskqueue, schedpicktask
.extern vmpagefault, kbdtrap, mousetrap
.extern	tasksave, taskjmp
#if (APIC)
.extern	mpapic
#endif

.text	32

trapde:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPDE, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPDE
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapdb:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPDB, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPDB
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapnmi:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPNMI, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPNMI
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapbp:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPBP, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPBP
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapof:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPOF, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPOF
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapbr:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPBR, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPBR
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapud:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPUD, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPUD
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapnm:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPNM, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPNM
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapdf:
	_trapentererr
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPDF, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPDF
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret

trapts:
	_trapentererr
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPTS, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPTS
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret
trapnp:
	_trapentererr
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPNP, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPNP
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret

trapss:
	_trapentererr
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPSS, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPSS
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret

trapgp:
	_trapentererr
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPGP, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPGP
	pushl	$0x00000000
#endif
	call	sigfunc

_gpfin:	
	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret

trappf:
	_trapentererr
	movl	%esp, %ebp

#if (FASTINTR)
	/* error code is in %ecx */
	movl	%cr2, %edx		// fault address
	xorl	%eax, %eax		// FIXME: PID
#else
	movl	%cr2, %edx		// linear address
	pushl	%ecx			// error code
	pushl	%edx
	pushl	$0x00000000		// FIXME: push PID here
#endif	
	call	vmpagefault

	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret

trapmf:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPMF, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPMF
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret

trapac:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPAC, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPAC
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleaveerr
	sti
	iret

trapmc:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPMC, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPMC
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
	sti
	iret

trapxf:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPXF, %ebx
	xorl	%eax, %eax
#else
	pushl	%ecx
	pushl	$TRAPXF
	pushl	$0x00000000
#endif
	call	sigfunc

	movl	%ebp, %esp
	_trapleave
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
	/* load [per-CPU] task structure address */
	movl	%gs:8, %ecx
	leal	580(%ecx), %esp
	/* load %ebx original value */
	movl	(%ebp), %ecx
	/* push general purpose registers into task control block */
	pushal
	/* restore frame pointer */
	leal	4(%ebp), %esp
	popl	%ebp
	/* arguments for schedpicktask */
	movl	%ebp, %ecx		// frame pointer
	movl	(%esp), %edx		// return address
	movl	%gs:8, %eax		// task structure address
	/* schedpicktask() returns task ID in eax */
	movl	schedpicktask, %ebx
	call	*%ebx
	/* task ID argument in eax */
	call	taskjmp

irqkbd:
	_trapenter
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
	_trapleave
	sti
	iret

irq3:
	_trapenter
	movl	%esp, %ebp

#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	
	movl	%ebp, %esp
	_trapleave
	sti
	iret

irq4:
	_trapenter
	movl	%esp, %ebp

#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	
	movl	%ebp, %esp
	_trapleave
	sti
	iret

irq5:
	_trapenter
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
	_trapleave
	sti
	iret

irqmouse:
	_trapenter
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
	_trapleave
	sti
	iret

#if (SMP) || (APIC)

irqerror:
	sti
	iret

irqspurious:
	_trapenter
	movl	%esp, %ebp

	call	mpspurint

	movl	%ebp, %esp
	_trapleave
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

