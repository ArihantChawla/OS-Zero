#define __ASSEMBLY__ 1
#include <kern/unit/ia32/boot.h>

.globl	_start, start
.globl	kernidt, kernpmaplvl4

.text	32

_start:
start:
	cli
	jmp	_start2

mboothdr:
.long	_MBMAGIC
.long	_MBFLAGS
.long	_MBCHKSUM

_start2:
	movl	$KERNSTKTOP, %esp
	movl	%esp, %ebp
	call	kmain

.align NBPG

kernidt:
	.space	NBPG
kernpmaplvl4:
	.space	NBPG

