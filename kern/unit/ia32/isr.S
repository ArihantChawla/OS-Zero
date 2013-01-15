#define __ASSEMBLY__ 1
#include <kern/conf.h>
#include <kern/unit/x86/trap.h>

/*
 * NOTE: we use interrupt gates, which automatically disable further
 * interrupts.
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
 * IRQ routines need to end with intfinirq(i).
 * interrupt routines with error must adjust stack pointer.
 * in the end, we enable interrupts with STI.
 */

.globl	trapde, trapdb, trapnmi, trapbp, trapof, trapbr, trapud, trapnm, trapdf
.globl	trapts, trapnp, trapss, trapgp, trappf, trapmf, trapac, trapmc, trapxf
.globl	irqtimer0, irqtimer, irqkbd, irqmouse, irqerror, irqspurious
#if (VBE2DRV)
.globl	trapv86, v86csel, v86ip, v86sp
#endif
.globl	irqvec

.extern	sigfunc, tmrsyncscr, thryield, vmpagefault, kbdtrap, mousetrap
#if (SMP)
.extern	mpspurint
#endif
.extern v86func

.text	32

trapde:
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
#if 0
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
#endif
	sti
	iret

trapnmi:
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
	pusha
	movl	32(%esp), %eax
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
	popal
	addl	$4, %esp
	sti
	iret

trapts:
	pushal
	movl	32(%esp), %eax
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
	pushal
	movl	32(%esp), %eax
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
	pushal
	movl	32(%esp), %eax
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
	pushal
	movl	32(%esp), %eax
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	$TRAPGP
	call	sigfunc

_gpfin:	
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
	pushal
	movl	32(%esp), %eax
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	pushl	%eax			// error code
	movl	%cr2, %eax		// linear address
	pushl	%eax
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
	pushal
	movl	32(%esp), %eax
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
	pushal
	movl	32(%esp), %eax
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
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

	movl	$irqvec, %eax
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
	pushal
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	movl	%esp, %ebp

//	call	tmrsyncscr
//	call	tmrfunc
	call	thryield

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

irqkbd:
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

#if (SMP)
irqerror:
	sti
	iret

irqspurious:
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

