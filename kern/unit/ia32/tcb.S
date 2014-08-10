#define __ASSEMBLY__ 1
#include <kern/conf.h>

#if (ASMSWITCH)

.globl	m_tcbsave, m_tcbjmp

.text	32

.align	32

m_tcbsave:
	/* prologue */
	pushl	%ebp
	movl	%esp, %ebp
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
	/* push segment registers */
//	pushl	%gs
	pushl	%fs
	pushl	%es
	pushl	%ds
	pushl	%ss
	subl	$4, %esp
	pushfl
	pushl	%cs
	/* caller sets return address */
//	movl	4(%ebp), %eax
//	pushl	%eax
	movl	%ebp, %esp
	popl	%ebp
	ret

.align	32

m_tcbjmp:
//	movl	8(%ebp), %eax
	leal	544(%eax), %esp
	popl	%eax
	movl	%eax, %cr3
	popal
	subl	$48, %esp
	popl	%ds
	popl	%es
	popl	%fs
//	popl	%gs
	subl	$32, %esp
	iret

#endif /* ASMSWITCH */

