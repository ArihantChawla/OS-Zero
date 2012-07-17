/*
 * this code relies on the caller to wait for the CPU to be marked active
 * - it's not reentrant
 *
 * mpentry is located at 0x9f000
 * GDT is located at MPGDT
 * call stack is located at MPENTRYSTK
 * processor kernel stack is located at MPSTKSIZE + cpu->apicid * MPSTKSIZE
 *
 * call stack
 * ----------
 *
 * 0x9e000	- stack top
 * 0x9dffc	- APIC ID
 * 0x9dff8	- processor kernel stack address
 * 0x9dff4	- page directory address
 */

#include <kern/conf.h>

#if (SMP)

#define __ASSEMBLY__ 1
#include <kern/ia32/boot.h>
#include <kern/ia32/mp.h>

.text	16

mpentry:
	/* disable interrupts */
	cli
	/* set data segment */
	movw	$0x9e00, %ax
	movw	%ax, %ds
	/* set up segmentation */
	.byte	0x66
	lgdt	(MPGDT)
//	.byte	0x66, 0x0f, 0x01, 0x15, 0x00, 0xe0, 0x09, 0x00
	/* switch to protected mode */
	movl	%cr0, %eax
	orl	$CR0PE, %eax
	movl	%eax, %cr0

.text	32
_mpentry32:
	/* initialise %cs */
//	.byte	0x66
	ljmp	$TEXTSEL, $(_mpflush - mpentry + 0x9f000)
_mpflush:
	/* load other segment registers */
	movw	$DATASEL, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	/* set the stack up */
//	movw	$STKSEL, %ax
	movw	%ax, %ss
	movl	$(MPENTRYSTK - 8), %esp
//	movl	$_mpdone, %ecx
	/* load PDBR and enable paging */
	popl	%eax			// page directory address
	movl	%eax, %cr3
	movl	%cr0, %eax
	orl	$CR0PG, %eax
	movl	%eax, %cr0
	popl	%eax			// kernel stack address
	popl	%ebx			// APIC ID
	/* initialise processor kernel stack */
	movl	%eax, %esp
	/* the stack has the APIC ID as an argument */
	pushl	%ebx
	call	mpmain
//	jmp	*%ecx
//_mpdone:
//	ret
	
#endif /* SMP */

