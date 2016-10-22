#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/bits/cpu.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/macro.S>
#include <kern/unit/ia32/task.h>

	/* TODO: privilege transition(?), per-cpu segment (%gs), tss/ltr */

.globl	m_taskjmp

.text	32

.align	32

	/* FASTCALL NORETURN void m_taskjmp(struct m_task *task); */

m_taskjmp:
	movl	%gs:8, %eax
	/* fetch fxsave-word */
	movl	(%eax), %ebx
	/* adjust %esp to task->pdbr */
	leal	4(%eax), %esp
	andl	$CPUHASFXSR, %ebx
	leal	128(%eax), %edx
	cmpl	$0, %ebx
	je	_frstor
	fxrstor	(%edx)
	jmp	_fpudone
_frstor:
	frstor	(%edx)
_fpudone:
	fwait
	leal	4(%eax), %esp
	/* fetch page directory page register */
	movl	52(%eax), %ebx
	/* load address of task->segregs into %esp */
	popl	%ds
	popl	%es
	popl	%fs
	popl	%gs
	/* restore page directory page register */
	movl	%ebx, %cr3
	/* pop general purpose registers */
	popal
	/* - stack pointer points to top of segregs */
	addl	$12, %esp
#if (APIC)
	_apiceoi
#else
	_piceoi1
#endif
	iretl

