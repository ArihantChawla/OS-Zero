#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/trap.h>
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

.extern	sigfunc, tmrsyncscr, schedpicktask
.extern vmpagefault, kbdtrap, mousetrap
#if (APIC)
.extern	mpapic
#endif

.text	32

trapde:
	_trapenter
	movl	%esp, %ebp

#if (FASTINTR)
	movl	$TRAPDE, %edx
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
	movl	$TRAPDB, %edx
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
	movl	$TRAPNMI, %edx
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
	movl	$TRAPBP, %edx
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
	movl	$TRAPOF, %edx
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
	movl	$TRAPBR, %edx
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
	movl	$TRAPUD, %edx
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
	movl	$TRAPNM, %edx
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
	movl	$TRAPDF, %edx
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
	movl	$TRAPTS, %edx
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
	movl	$TRAPNP, %edx
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
	movl	$TRAPSS, %edx
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
	movl	$TRAPGP, %edx
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
	xorl	%gs:16, %eax		// FIXME: PID
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
	movl	$TRAPMF, %edx
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
	movl	$TRAPAC, %edx
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
	movl	$TRAPMC, %edx
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
	movl	$TRAPXF, %edx
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

	/* 32-bit x86 context switch */
irqtmr:
	pushl	%ebp			// push frame pointer
	movl	%esp, %ebp		// store stack pointer
	pushl	%eax			// back %eax up
	movl	%gs:8, %eax		// per-cpu current task
	leal	568(%eax), %esp		// adjust sp for m_tcb.genregs
	pushal				// push general-purpose registers
	movl	-24(%esp), %ebx		// fetch fxsave-flag
	cmpl	$0, %ebx
	jne	_fxsave			// use FXSAVE if non-zero
	fnsave	(%eax)			// use FNSAVE if zero
	jmp	_fpudone
_fxsave:	
	fxsave	(%eax)
_fpudone:
	fwait
	/* push segment registers */
	pushl	%gs			// kernel thread-local storage
	pushl	%fs			// user thread-local storage
	pushl	%es			// buffer segment? something... :)
	pushl	%ds			// data segment
	movl	8(%ebp), %edx		// fetch pushed %cs
	movl	-4(%ebp), %eax		// retrieve %eax
	movl	%cr3, %ecx		// store %cr3 (page directory address)
	cmpl	$UTEXTSEL, %edx		// did we come from userland?
	jne	_getsysstk		// stack-info was built if not
	movl	16(%ebp), %ebx		// fetch stored stack pointer
	movl	20(%ebp), %edi		// fetch stored %ss
	addl	$24, %ebx		// adjust for past the trap frame
	jmp	_havestk
_getsysstk:
	/* construct stack information */
	movl	%ebp, %ebx
	movl	%ss, %edi
	addl	$16, %ebx
_havestk:	
	movl	4(%ebp), %esi		// pushed return address
	movl	%ecx, -4(%esp)		// store page directory base register
	movl	%eax, 44(%esp)		// patch %eax in m_tcb.genregs
	movl	%edi, 64(%esp)		// store %ss in m_tcb.iret
#if 0
	movl	%ebx, 24(%esp)		// patch stack pointer
#endif
	leal	48(%esp), %eax		// store m_tcb.iret address
	movl	%edx, 52(%esp)		// store %cs in m_tcb.iret
	movl	%ebx, 28(%esp)		// patch frame pointer in m_tcb.genregs
	movl	12(%ebp), %edi		// fetch pushed flags register
	movl	%ebx, 60(%esp)		// store caller stack pointer for iret
	movl	%esi, 48(%esp)		// store return address for iret
	movl	%edi, 56(%esp)		// store pre-interrupt flags for iret
	movl	%eax, 24(%esp)		// patch stack pointer in m_tcb.genregs
	/* schedule next thread */
	movl	%gs:8, %eax		// task structure address argument
	movl	schedpicktask, %ebx	// get scheduler-specific function
	movl	%ebp, %esp		// restore stack pointer
	call	*%ebx			// call scheduler function (taskpick)
	/* schedpicktask leaves the address of the task/context in %eax */
	call	m_tcbjmp		// launch next thread

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

