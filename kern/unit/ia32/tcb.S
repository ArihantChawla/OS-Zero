#define __ASSEMBLY__ 1
#include <kern/conf.h>

#if (ASMSWITCH)

	/* TODO: stack switches, per-cpu segment (%gs) */

.globl	m_tcbsave, m_tcbjmp

.text	32

.align	32

	/* ASMLINK void m_tcbsave(struct m_tcb *tcb); */

m_tcbsave:
	/* prologue */
	pushl	%ebp
	movl	%esp, %ebp
	/* load tcb-argument into %esp */
	movl	8(%ebp), %esp
	/* save FPU context */
	fnsave	(%esp)
	/* push general purpose registers */
	addl	$580, %esp
	pushal
	/* save caller stack pointer in m_pusha */
	leal	-8(%ebp), %eax
	movl	%eax, 8(%esp)
	/* save caller stack pointer for iret */
	movl	%eax, -24(%esp)
	/* save caller frame pointer in m_pusha */
	leal	(%ebp), %eax
	movl	%eax, 12(%esp)
	/* save page directory base register */
	movl	%cr3, %ebx
	subl	$4, %esp
	movl	%ebx, (%esp)
	/* push segment registers; %gs is per-CPU and loaded in thrjmp */
	pushl	%fs
	pushl	%es
	pushl	%ds
	/* construct return frame for iret; return address stored in thrsave */
	pushl	%ss
	/* push %eflags */
	subl	$4, %esp
	pushfl
	pushl	%cs
	/* epilogue */
	movl	%ebp, %esp
	popl	%ebp
	ret

.align	32

	/* FASTCALL void m_tcbjmp(struct m_tcb *tcb); */

m_tcbjmp:
	/* load &tcb->pdbr into %esp */
	leal	544(%eax), %esp
	/* retrieve and set page directory page register */
	popl	%eax
	movl	%eax, %cr3
	/* pop general purpose registers */
	popal
	/* adjust %esp to &tcb->segregs */
	subl	$48, %esp
	popl	%ds
	popl	%es
	popl	%fs
//	popl	%gs
	/* adjust %esp to &tcb->iret */ 
	subl	$32, %esp
	iret

#endif /* ASMSWITCH */

