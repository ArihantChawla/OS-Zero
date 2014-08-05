#define __ASSEMBLY__ 1
#include <kern/conf.h>

#if (ASMSWITCH)

.globl	m_tcbsave, m_tcbjmp

.text	32

.align	32

m_tcbsave:
	pushl	%eax
	movl	4(%esp), %eax
	fnsave	(%eax)
	popl	%eax
	leal	584(%esp), %esp
	pushal
	movl	%cr3, %eax
	pushl	%eax
	pushl	%gs
	pushl	%fs
	pushl	%es
	pushl	%ds
	pushl	%ss
	pushl	%ebp
	pushfl
	pushl	%cs
	movl	4(%ebp), %eax
	pushl	%eax
	ret

.align	32

m_tcbjmp:	
	popl	%esp
	frstor	(%esp)
	movl	560(%esp), %esp
	leal	532(%esp), %esp
	popl	%ds
	popl	%es
	popl	%fs
	popl	%gs
	leal	512(%esp), %esp
	movl	%eax, %cr3
	iret

#endif /* ASMSWITCH */

