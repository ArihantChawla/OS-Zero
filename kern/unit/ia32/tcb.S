#define __ASSEMBLY__ 1
#include <kern/conf.h>

#if (ASMSWITCH)

.globl	m_tcbsave, m_tcbjmp

.text	32

.align	32

m_tcbsave:
	/* prologue */
	pushl	%ebp			// push frame pointer
	movl	%ebp, %esp		// set stack pointer to frame pointer
	/* push callee save registers */
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	/* store thread control block */
	movl	8(%ebp), %esp		// function argument
	fnsave	(%esp)			// save FPU context
	leal	584(%esp), %esp		// set stack pointer past genregs
	pushal				// push general purpose registers
	movl	%cr3, %ebx		// fetch page directry page register
	pushl	%ebx			// push PDBR
	/* push segment registers */
	pushl	%gs
	pushl	%fs
	pushl	%es
	pushl	%ds
	/* construct IRET frame */
	pushl	%ss
	movl	(%ebp), %ebx		// caller frame pointer
	pushl	%ebx
	pushfl
	pushl	%cs
	/* caller sets return address */
//	movl	4(%ebp), %ebx
//	pushl	%ebx
	leal	-12(%ebp), %esp
	popl	%esi
	popl	%edi
	popl	%ebx
	popl	%ebp
	ret

.align	32

m_tcbjmp:
	movl	4(%ebp), %ebx
	leal	548(%ebx), %esp
	popl	%ebx
	movl	%ebx, %cr3
	popal
	subl	$52, (%esp)
	popl	%ds
	popl	%es
	popl	%fs
	popl	%gs
	subl	$36, (%esp)
	iret

#endif /* ASMSWITCH */

