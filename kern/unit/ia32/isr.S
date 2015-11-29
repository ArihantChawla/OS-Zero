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
	iretl

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
	iretl

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
	iretl

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
	iretl

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
	iretl

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
	iretl

trapud:
	_trapenter
	movl	%esp, %ebp

	movl	8(%esp), %eax	// old ESP
	movl	(%eax), %ecx	// stored EIP
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
	iretl

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
	iretl

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
	iretl

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
	iretl
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
	iretl

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
	iretl

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
	iretl

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
	iretl

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
	iretl

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
	iretl

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
	iretl

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
	iretl

#if (APIC)
	/* dummy interrupt handler for probing APIC frequency */
irqtmrcnt:
	_apiceoi
	iretl
#endif

	/*
	 * simple interface to let the PIT do sleep() and such actions early in
	 * the bootstrap process
	 */
irqtmr0:
	/* push general purpose registers */
	_trapenter
//	pushal
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
	iretl

	/* 32-bit x86 context switch */
	/*
	 * registers after interrupt
	 * -------------------------
	 * ebp		- old ESP _and_ EBP
	 *
	 * top of stack after interrupt
	 * ----------------------------
	 * ss		- user-stack segment selector
	 * esp		- userland stack pointer
	 * eflags
	 * cs
	 * eip
	 * ebp		<- EBP
	 * eax
	 *
	 * NOTE: ss and esp are only pushed if the interrupt happened
	 * in user-mode, i.e. there's a privilege transition to system-mode
 	 */
irqtmr:
	pushl	%ebp			// push frame pointer
	movl	%esp, %ebp		// store stack pointer
	pushl	%eax			// back %eax up
	movl	%gs:8, %eax		// per-cpu current task
	leal	568(%eax), %esp		// adjust sp for m_tcb.genregs
	pushal				// push general-purpose registers
	/* store FPU context */
	movl	-24(%ebp), %ebx		// fetch fxsave-flag
	cmpl	$0, %ebx
	jne	_fxsave			// use FXSAVE if non-zero
	fnsave	(%eax)			// use FNSAVE if zero
	jmp	_fpudone
_fxsave:	
	fxsave	(%eax)
_fpudone:
	//
	/* push segment registers */
	pushl	%gs			// kernel thread-local storage
	pushl	%fs			// user thread-local storage
	pushl	%es			// buffer segment? something... :)
	pushl	%ds			// data segment
	movl	8(%ebp), %edx		// fetch pushed %cs
	movl	-4(%ebp), %eax		// retrieve %eax
	movl	%cr3, %ecx		// get %cr3 (page directory address)
	cmpl	$UTEXTSEL, %edx		// did we come from userland?
	jne	_getsysstk		// stack-info was not pushed if not
	movl	(%ebp), %ebx		// fetch stored frame pointer
	movl	16(%ebp), %edi		// fetch stored stack pointer
	movl	$UDATASEL, %esi
	movl	%ebx, 28(%esp)		// patch frame pointer in m_tcb.genregs
	movl	%esi, 60(%esp)		// caller stack pointer in m_tcb.iret
	jmp	_havestk
_getsysstk:
	/* construct stack information */
	movl	(%ebp), %ebx		// get after-interrupt frame pointer
	movl	(%ebp), %esi
	movl	$DATASEL, %edi
	addl	$4, %ebx		// adjust to caller stack pointer
	addl	$8, %esi		// adjust to caller frame pointer
	movl	%ebx, 60(%esp)		// caller stack pointer in m_tcb.iret
	movl	%esi, 28(%esp)		// patch frame pointer in m_tcb.genregs
_havestk:	
	fwait	
	movl	4(%ebp), %esi		// pushed return address
	movl	%ecx, -4(%esp)		// store page directory base register
	movl	%eax, 44(%esp)		// patch %eax in m_tcb.genregs
	movl	%edi, 64(%esp)		// store %ss in m_tcb.iret
	addl	$12, %ebx		// adjust stack pointer past trap frame
	movl	(%ebp), %ecx		// pushed frame pointer
	movl	%edx, 52(%esp)		// store %cs in m_tcb.iret
	movl	12(%ebp), %edi		// fetch pushed flags register
	movl	%esi, 48(%esp)		// store return address for iret
	movl	%edi, 56(%esp)		// store pre-interrupt flags for iret
	/* schedule next thread */
	movl	%gs:8, %eax		// task structure address argument
	movl	schedpicktask, %ebx	// get scheduler-specific function
	leal	4(%ebp), %esp		// restore stack pointer
	pushl	%eax			// store copy of task pointer
	call	*%ebx			// call scheduler function (taskpick)
	popl	%ecx
	cmpl	%eax, %ecx		// did we reschedule the same task?
	jne	_swtch			// if not, branch to _swtch
	/* continue the same task */
	leal	536(%eax), %esp
	popal
	leal	4(%ebp), %esp		// restore stack pointer
	addl	$16, %ebp		// adjust frame pointer
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	iretl
_swtch:
	call	m_tcbjmp
	
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
	iretl

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
	iretl

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
	iretl

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
	iretl

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
	iretl

#if (SMP) || (APIC)

irqerror:
	sti
	iretl

irqspurious:
	_trapenter
	movl	%esp, %ebp

	call	mpspurint

	movl	%ebp, %esp
	_trapleave
	sti
	iretl

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

