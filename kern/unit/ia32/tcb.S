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
	/* fetch fxsave-word */
	movl	512(%eax), %ebx
	/* adjust %esp to tcb->pdbr */
	leal	516(%eax), %esp
	cmpl	$0, %ebx
	je	_frstor
	fxrstor	(%eax)
	jmp	_fpudone
_frstor:
	frstor	(%eax)
_fpudone:
	/* restore page directory page register */
	popl	%ebx
	movl	%ebx, %cr3
	/* load address of tcb->segregs into %esp */
	popl	%ds
	popl	%es
	popl	%fs
	popl	%gs
	fwait
	/*
	 * pop general purpose registers
	 * - stack pointer points to m_tcb->iret
	 */

	movl	%eax, %gs:8		// k_curtask
	movl	528(%eax), %ebx		// k_curproc
	movl	548(%eax), %ecx		// k_curpid
	movl	%ebx, %gs:4
	movl	%ecx, %gs:16
	
	popal
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	iretl

