#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/macro.S>
#include <kern/unit/ia32/tcb.h>

	/* TODO: privilege transition(?), per-cpu segment (%gs), tss/ltr */

.globl	m_tcbjmp

.text	32

.align	32

	/* FASTCALL NORETURN void m_tcbjmp(struct m_tcb *tcb); */

m_tcbjmp:
	/* fetch fxsave-word */
	movl	576(%eax), %ebx
	/* adjust %esp to tcb->pdbr */
	leal	580(%eax), %esp
	andl	$M_FPXMM, %ebx
	cmpl	$0, %ebx
	movl	%esp, %ecx
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
	/* set up per-cpu variables */
	movl	%eax, %gs:8		// k_curtask
	movl	100(%eax), %ebx		// k_curproc
	movl	120(%eax), %ecx		// k_curpid
	movl	%ebx, %gs:4
	movl	%ecx, %gs:16
	/* pop general purpose registers */
	popal
	/* - stack pointer points to m_tcb->trapnum */
	addl	$8, %esp
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	iretl

