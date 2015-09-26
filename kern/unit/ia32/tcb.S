#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/macro.S>

	/* TODO: privilege transition(?), per-cpu segment (%gs), tss/ltr */

.globl	m_tcbjmp

.text	32

.align	32

	/* FASTCALL void m_tcbjmp(struct m_tcb *tcb); */

m_tcbjmp:
	/* adjust %esp to tcb->segregs */
	leal	532(%eax), %esp
	popl	%ds
	popl	%es
	popl	%fs
	popl	%gs
	/* load address of tcb->pdbr into %esp */
	/* restore page directory page register */
	popl	%eax
	movl	%eax, %cr3
	/* pop general purpose registers */
	popal
	/* adjust %esp to tcb->iret */ 
	subl	$72, %esp
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	iret

