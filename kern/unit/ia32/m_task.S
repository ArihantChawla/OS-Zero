#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/macro.S>
#include <kern/unit/ia32/task.h>

	/* TODO: privilege transition(?), per-cpu segment (%gs), tss/ltr */

.globl	m_taskjmp

.text	32

.align	32

	/* FASTCALL NORETURN void m_taskjmp(struct m_task *task); */

m_taskjmp:
	/* fetch fxsave-word */
	movl	(%eax), %ebx
	/* adjust %esp to task->pdbr */
	leal	4(%eax), %esp
	andl	$M_FPXMM, %ebx
	leal	88(%eax), %edx
	movl	%esp, %ecx
	cmpl	$0, %ebx
	je	_frstor
	fxrstor	(%edx)
	jmp	_fpudone
_frstor:
	frstor	(%edx)
_fpudone:
	/* fetch page directory page register */
	popl	%ebx
	/* load address of task->segregs into %esp */
	popl	%ds
	popl	%es
	popl	%fs
	popl	%gs
	/* restore page directory page register */
	movl	%ebx, %cr3
	fwait
	/* set up per-cpu variables */
	movl	%eax, %gs:8		// k_curtask
	movl	108(%eax), %ebx		// k_curproc
	movl	128(%eax), %ecx		// k_curpid
	movl	%ebx, %gs:4
	movl	%ecx, %gs:16
	/* pop general purpose registers */
	popal
	/* - stack pointer points to top of segregs */
	addl	$48, %esp
#if (APIC)
	_apiceoi
#else
	_piteoi1
#endif
	iretl

