#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/macro.S>

	/* TODO: privilege transition(?), per-cpu segment (%gs), tss/ltr */

.globl	m_tcbsave, m_tcbjmp

.text	32

.align	32

	/* ASMLINK void m_tcbsave(struct m_tcb *tcb); */
	/* NOTE: caller (timer interrupt handler) has already done PUSHA */

m_tcbsave:
	/* prologue */
	pushl	%ebp
	movl	%esp, %ebp
	/* load tcb-argument into %esp */
	movl	8(%ebp), %esp
	/* push general purpose registers */
	addl	$548, %esp
	movl	(%ebp), %eax
	movl	%eax, 12(%esp)
	/* save page directory base register */
	movl	%cr3, %edx
	pushl	%edx
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
	/* load tcb->pdbr into %esp */
	leal	544(%eax), %esp
	/* retrieve and set page directory page register */
	popl	%eax
	movl	%eax, %cr3
	/* pop general purpose registers */
	popal
	/* adjust %esp to tcb->segregs */
	subl	$48, %esp
	popl	%ds
	popl	%es
	popl	%fs
//	popl	%gs
	/* adjust %esp to tcb->iret */ 
	subl	$32, %esp
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
//	sti
	iret

