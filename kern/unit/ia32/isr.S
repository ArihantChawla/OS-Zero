#define __ASSEMBLY__ 1
#include <kern/conf.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/boot.h>

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

.globl	trapde, trapdb, trapnmi, trapbp, trapof, trapbr, trapud, trapnm, trapdf
.globl	trapts, trapnp, trapss, trapgp, trappf, trapmf, trapac, trapmc, trapxf
.globl	irqtmr0, irqtmr, irqkbd, irqmouse, irqerror, irqspurious
.globl	irqvec
#if (SMP) || (APIC)
.globl	irqerror, irqspurious, mpspurint, irqtmrcnt
#endif

.extern	sigfunc, tmrsyncscr, schedyield, vmpagefault, kbdtrap, mousetrap
.extern	thrsave, thrjmp
#if (APIC)
.extern	mpapic
#endif

.text	32

trapde:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPDE
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapdb:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPDB
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapnmi:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPNMI
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapbp:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPBP
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapof:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPOF
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapbr:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPBR
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapud:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPUD
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapnm:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPNM
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapdf:
	pusha
	popl	%eax
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPDF
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	addl	$4, %esp
	popal
	sti
	iret

trapts:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPTS
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret
trapnp:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPNP
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapss:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPSS
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapgp:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPGP
	call	sigfunc

_gpfin:	
	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trappf:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	%eax			// error code
	movl	%cr2, %eax		// linear address
	pushl	%eax
	pushl	$0x00000000		// TODO: push PID here
	call	vmpagefault

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapmf:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPMF
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapac:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPAC
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	addl	$4, %esp
	sti
	iret

trapmc:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPMC
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

trapxf:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	pushl	$TRAPXF
	call	sigfunc

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

#if (APIC)
	/* dummy interrupt handler for probing APIC frequency */
irqtmrcnt:
	/* send APIC EOI */
	movl	$0x00000000, 0xfee000b0
	iret
#endif

irqtmr0:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
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
	popl	%fs
	popl	%es
	popl	%ds
	popal
	movl	$1, irqtmrfired
	sti
	iret
	
irqtmr:
//	movl	%esp, %ebp
//	movl	%ebp, %esp
	/* arguments for thrsave */
	movl	%esp, %ecx		// frame pointer
	addl	$20, %ecx
	movl	(%esp), %edx		// return address
	movl	%gs:8, %eax		// thread structure
	call	thrsave
	/* schedyield returns thread ID in eax */
	movl	schedyield, %ebx
	call	*%ebx
	/* thread ID argument in eax */
	call	thrjmp

irqkbd:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	4(%eax), %ebx
	jz	_irqkbddone
	call	*%ebx
_irqkbddone:	
#if (APIC)
	movl	$0x00000000, 0xfee000b0
#else
	movb	$0x20, %al
	outb	%al, $0x20
#endif
	
	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irq3:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

#if (APIC)
	movl	$0x00000000, 0xfee000b0
#else
	movb	$0x20, %al
	outb	%al, $0x20
#endif
	
	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irq4:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

#if (APIC)
	movl	$0x00000000, 0xfee000b0
#else
	movb	$0x20, %al
	outb	%al, $0x20
#endif
	
	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irq5:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	20(%eax), %ebx
	jz	_irq5done
	call	*%ebx
_irq5done:	
#if (APIC)
	movl	$0x00000000, 0xfee000b0
#else
	movb	$0x20, %al
	outb	%al, $0x20
#endif
	
	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
	sti
	iret

irqmouse:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	movl	$irqvec, %eax
	movl	48(%eax), %ebx
	jz	_irqmousedone
	call	*%ebx
_irqmousedone:	
#if (APIC)
	movl	$0x00000000, 0xfee000b0
#else
	movb	$0x20, %al
	outb	%al, $0xa0
	outb	%al, $0x20
#endif
	
	movl	%ebp, %esp
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
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	movl	%esp, %ebp

	call	mpspurint

	movl	%ebp, %esp
	popl	%fs
	popl	%es
	popl	%ds
	popal
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

