#include <kern/conf.h>

#if (VBE)

#define __ASSEMBLY__ 1
#include <kern/unit/ia32/boot.h>

.globl	realstart, realend, realint10
.extern	idtptr

.text

.code32

.align	4

realint10:
	pushl	%ebp
	movl	%esp, %ebp
	pushfl
	cli
	pushal
	movl	%esp, _espsave
	cld
	ljmpl	$REALCODESEL, $KERNREALBASE

.align	4

.code16
realstart:
	/* load NULL IDT */
	lidtl	(_realidtptr - realstart + KERNREALBASE)
	/* set up 16-bit segments */
	movw	$REALDATASEL, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss
	/* clear protected mode bit */
	mov	%cr0, %eax
	and	$(~(CR0PE | CR0PG)), %eax
	mov	%eax, %cr0
	/* jump to real mode code */
	ljmpl	$0x0000, $(_real16 - realstart + KERNREALBASE)

.align	4
	
_real16:
	/* zero segment registers */
	xorw	%ax, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss
	movw	$(KERNREALSTK - 16), %sp
	/* load real mode IDT */
	movw	$0x3ff, _realidtptr - realstart + KERNREALBASE
	lidtl	(_realidtptr - realstart + KERNREALBASE)
	sti
	popa
	int	$0x10
	pusha
	cli
	mov	%cr0, %eax
	orl	$(CR0PE), %eax
	mov	%eax, %cr0
	ljmpl	$0x08, $(_protmode - realstart + KERNREALBASE)

.align	4

.code32
_protmode:
	/* set up segment registers */
	movw	$DATASEL, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss
	movl	_espsave, %esp
	popal
	popfl
	/* leave */
	movl	%ebp, %esp
	popl	%ebp
	lidt	(idtptr)
	retl

.align	16

#if 0
_realgdt:
	/* NULL */
	.short	0, 0
	.byte	0, 0, 0, 0
	/* ring 0 32-bit code descriptor */
	.short	0xffff, 0
	.byte	0, 0x98, 0xcf, 0
	/* ring 0 32-bit data descriptor */
	.short	0xffff, 0
	.byte	0, 0x92, 0xcf, 0

_realgdtdesc:
	.short	_realgdtdesc - _realgdt - 1
	.long	_realgdt - realstart + KERNREALBASE
#endif
_espsave:
	.long	0x00000000
_realidtptr:
	.short	0x0000
	.long	0x00000000

.align	4
	
realend:

#endif /* VBE */

