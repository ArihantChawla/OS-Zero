#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/macro.S>

	/* TODO: privilege transition(?), per-cpu segment (%gs), tss/ltr */

.globl	m_tcbjmp

.text	32

.align	32

	/* FASTCALL NORETURN void m_tcbjmp(struct m_tcb *tcb); */

m_tcbjmp:
	/* adjust %esp to tcb->segregs */
	movl	512(%eax), %ebx
	leal	516(%eax), %esp
	cmpl	$0, %ebx
	je	_frstor
	fxrstor	(%eax)
	jmp	_fpudone
_frstor:
	frstor	(%eax)
_fpudone:
	fwait
	/* restore page directory page register */
	popl	%ebx
	movl	%ebx, %cr3
	/* load address of tcb->segregs into %esp */
	popl	%ds
	popl	%es
	popl	%fs
	popl	%gs
	/* pop general purpose registers */
	popal
#if 0
	movl	%gs:8, %esp
	addl	$568, %esp
#endif
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	iret

