#include <kern/conf.h>

#if (VBE)

#define __ASSEMBLY__ 1
#include <kern/unit/ia32/boot.h>

.globl	realstart, realend, realint10
.extern	idtptr

.text

.code32

.align	4

	/* interrupt handler for 0x10 */
realint10:
	cli
	pushl	%ebp
	movl	%esp, %ebp
	pushfl
	pushal
	movl	%esp, _espsave
	cld
	ljmpl	$REALCODESEL, $KERNREALBASE

.align	4

.code16

	/* drop back to 16-bit real mode */
realstart:
#if (NEWCLI)
	cli
#endif
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
	and	$(~CR0PE), %eax
	mov	%eax, %cr0
	/* jump to real mode code */
	ljmpl	$0x0000, $(_real16 - realstart + KERNREALBASE)

.align	4

	/* generate interrupt 0x10 in real mode */
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
	popa
	sti
	int	$0x10
	cli
	pusha
	lgdtl	(_realgdtptr - realstart + KERNREALBASE)
	mov	%cr0, %eax
	orl	$(CR0PE), %eax
	mov	%eax, %cr0
	ljmpl	$0x08, $(_protmode - realstart + KERNREALBASE)

.align	4

.code32

	/* reinitialise 32-bit protected mode */
_protmode:
//	lgdtl	(gdtptr)
	/* set up segment registers */
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %ss
	movw	%ax, %gs
//	movw	$CPUSEL, %ax
//	movw	%ax, %gs
	movl	_espsave, %esp
	popal
	popfl
	/* leave */
	movl	%ebp, %esp
	popl	%ebp
	lidtl	(idtptr)
	ljmpl	$TEXTSEL, $(_protret - realstart + KERNREALBASE)
_protret:
	sti
	retl

.align	16

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

_realgdtptr:
	.short	_realgdtptr - _realgdt - 1
	.long	_realgdt - realstart + KERNREALBASE

.align	4
	
_espsave:
	.long	0x00000000

.align	8

_realidtptr:
	.short	0x0000
	.long	0x00000000

.align	4
	
realend:

#endif /* VBE */

